#include "GraphicsContext.hpp"

#include <GLFW/glfw3.h>

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "ToyEngine/Renderer/Device.hpp"
#include "ToyEngine/Renderer/Shader.hpp"
#include "ToyEngine/Renderer/SwapChain.hpp"
#include "ToyEngine/Renderer/VertexArray.hpp"

// instantiate the default dispatcher
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace TE {

GraphicsContext* GraphicsContext::instance = nullptr;

GraphicsContext::GraphicsContext(GLFWwindow* window)
    : device{window}, allocator{device}, swapchain{window, device} {
  assert(instance == nullptr);
  instance = this;

  // transient command pool
  vk::CommandPoolCreateInfo pool_info{
      .flags = vk::CommandPoolCreateFlagBits::eTransient,
      .queueFamilyIndex = device.getGraphicsQueueIndex(),
  };
  transient_command_pool = device.getDevice().createCommandPool(pool_info);

  createRenderPass();
  createDescriptorSets();
  createGraphicsPipeline();
  swapchain.createFramebuffers(render_pass);
  createFrameData();
}

GraphicsContext::~GraphicsContext() {
  auto device = this->device.getDevice();

  for (auto& frame : frame_data) {
    device.destroySemaphore(frame.acquire_semaphore);
    device.destroySemaphore(frame.release_semaphore);
    device.destroyFence(frame.submit_fence);
    device.destroyCommandPool(frame.command_pool);
  }

  if (graphics_pipeline) {
    device.destroyPipeline(graphics_pipeline);
  }

  if (pipeline_layout) {
    device.destroyPipelineLayout(pipeline_layout);
  }

  if (descriptor_pool) {
    device.destroyDescriptorPool(descriptor_pool);
  }

  if (descriptor_set_layout) {
    device.destroyDescriptorSetLayout(descriptor_set_layout);
  }

  if (render_pass) {
    device.destroyRenderPass(render_pass);
  }

  if (transient_command_pool) {
    device.destroyCommandPool(transient_command_pool);
  }

  instance = nullptr;
}

void GraphicsContext::beginFrame() {
  auto device = this->device.getDevice();
  auto& frame = frame_data[current_frame];

  (void)device.waitForFences(frame.submit_fence, true, UINT64_MAX);
  swapchain.acquireNextImage(frame.acquire_semaphore);
  device.resetFences(frame.submit_fence);
  device.resetCommandPool(frame.command_pool);

  vk::CommandBufferBeginInfo begin_info{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  frame.command_buffer.begin(begin_info);
}

void GraphicsContext::endFrame() {
  auto queue = this->device.getQueue();
  auto& frame = frame_data[current_frame];

  frame.command_buffer.end();

  vk::Semaphore wait_semaphores[] = {frame.acquire_semaphore};
  vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  vk::Semaphore signal_semaphores[] = {frame.release_semaphore};
  vk::SubmitInfo submit_info{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = wait_semaphores,
      .pWaitDstStageMask = wait_stages,
      .commandBufferCount = 1,
      .pCommandBuffers = &frame.command_buffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = signal_semaphores,
  };
  queue.submit(submit_info, frame.submit_fence);

  vk::SwapchainKHR swc = swapchain.get();
  uint32_t image = swapchain.getImage();

  vk::PresentInfoKHR present_info{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signal_semaphores,
      .swapchainCount = 1,
      .pSwapchains = &swc,
      .pImageIndices = &image,
  };
  (void)queue.presentKHR(present_info);

  current_frame = (current_frame + 1) % max_frames_in_flight;
}

void GraphicsContext::beginPass() {
  vk::ClearValue clear_value{{{{0.01f, 0.01f, 0.033f, 1.0f}}}};
  auto extent = swapchain.getExtent();

  vk::Viewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(extent.width),
      .height = static_cast<float>(extent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };

  vk::Rect2D scissor{{0, 0}, extent};

  vk::RenderPassBeginInfo render_pass_info{
      .renderPass = render_pass,
      .framebuffer = swapchain.getFramebuffer(),
      .renderArea = {{0, 0}, extent},
      .clearValueCount = 1,
      .pClearValues = &clear_value,
  };

  auto& frame = frame_data[current_frame];
  frame.command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
  frame.command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipeline);
  frame.command_buffer.setViewport(0, viewport);
  frame.command_buffer.setScissor(0, scissor);
}

void GraphicsContext::endPass() {
  auto& frame = frame_data[current_frame];
  frame.command_buffer.endRenderPass();
}

void GraphicsContext::createRenderPass() {
  vk::AttachmentDescription color_attachment{
      .format = swapchain.getFormat(),
      .samples = vk::SampleCountFlagBits::e1,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
      .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
      .initialLayout = vk::ImageLayout::eUndefined,
      .finalLayout = vk::ImageLayout::ePresentSrcKHR,
  };

  vk::AttachmentReference color_attachment_ref{
      .attachment = 0,
      .layout = vk::ImageLayout::eColorAttachmentOptimal,
  };

  vk::SubpassDescription subpass{
      .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment_ref,
  };

  vk::SubpassDependency dependency{
      .srcSubpass = vk::SubpassExternal,
      .dstSubpass = 0,
      .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
      .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
      .srcAccessMask = vk::AccessFlagBits::eNone,
      .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
  };

  vk::RenderPassCreateInfo render_pass_info{
      .attachmentCount = 1,
      .pAttachments = &color_attachment,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency,
  };

  render_pass = device.getDevice().createRenderPass(render_pass_info);
}

void GraphicsContext::createDescriptorSets() {
  std::array<vk::DescriptorSetLayoutBinding, 3> layout_bindings = {
      vk::DescriptorSetLayoutBinding{
          .binding = 0,
          .descriptorType = vk::DescriptorType::eUniformBuffer,
          .descriptorCount = UNIFORM_BUFFER_COUNT,
          .stageFlags = vk::ShaderStageFlagBits::eAll,
      },
      {
          .binding = 1,
          .descriptorType = vk::DescriptorType::eStorageBuffer,
          .descriptorCount = STORAGE_BUFFER_COUNT,
          .stageFlags = vk::ShaderStageFlagBits::eAll,
      },
      {
          .binding = 2,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = TEXTURE_COUNT,
          .stageFlags = vk::ShaderStageFlagBits::eAll,
      },
  };
  std::array<vk::DescriptorBindingFlags, layout_bindings.size()> layout_flags = {
      vk::DescriptorBindingFlagBits::ePartiallyBound |
          vk::DescriptorBindingFlagBits::eUpdateAfterBind,
      vk::DescriptorBindingFlagBits::ePartiallyBound |
          vk::DescriptorBindingFlagBits::eUpdateAfterBind,
      vk::DescriptorBindingFlagBits::ePartiallyBound |
          vk::DescriptorBindingFlagBits::eUpdateAfterBind,
  };
  vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags{
      .bindingCount = layout_flags.size(),
      .pBindingFlags = layout_flags.data(),
  };
  vk::DescriptorSetLayoutCreateInfo layout_info{
      .pNext = &binding_flags,
      .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
      .bindingCount = layout_bindings.size(),
      .pBindings = layout_bindings.data(),
  };
  descriptor_set_layout = device.getDevice().createDescriptorSetLayout(layout_info);

  std::array<vk::DescriptorPoolSize, layout_bindings.size()> pool_sizes = {
      vk::DescriptorPoolSize{
          .type = vk::DescriptorType::eUniformBuffer,
          .descriptorCount = UNIFORM_BUFFER_COUNT,
      },
      {
          .type = vk::DescriptorType::eStorageBuffer,
          .descriptorCount = STORAGE_BUFFER_COUNT,
      },
      {
          .type = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = TEXTURE_COUNT,
      },
  };
  vk::DescriptorPoolCreateInfo pool_info{
      .flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
      .maxSets = 1,
      .poolSizeCount = pool_sizes.size(),
      .pPoolSizes = pool_sizes.data(),
  };
  descriptor_pool = device.getDevice().createDescriptorPool(pool_info);

  vk::DescriptorSetAllocateInfo alloc_info{
      .descriptorPool = descriptor_pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptor_set_layout,
  };
  descriptor_set = device.getDevice().allocateDescriptorSets(alloc_info)[0];
}

void GraphicsContext::createGraphicsPipeline() {
  auto device = this->device.getDevice();

  std::array<vk::DynamicState, 2> dynamic_states{
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor,
  };

  vk::PipelineDynamicStateCreateInfo dynamic_state{
      .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data(),
  };

  vk::VertexInputBindingDescription binding_desc{
      .binding = 0,
      .stride = sizeof(VertexArray::VertexType),
      .inputRate = vk::VertexInputRate::eVertex,
  };

  std::array<vk::VertexInputAttributeDescription, 2> attribute_descs = {
      vk::VertexInputAttributeDescription{
          .location = 0,
          .binding = 0,
          .format = vk::Format::eR32G32Sfloat,
          .offset = offsetof(VertexArray::VertexType, pos),
      },
      {
          .location = 1,
          .binding = 0,
          .format = vk::Format::eR32G32Sfloat,
          .offset = offsetof(VertexArray::VertexType, uv),
      },
  };

  vk::PipelineVertexInputStateCreateInfo vertex_input{
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &binding_desc,
      .vertexAttributeDescriptionCount = attribute_descs.size(),
      .pVertexAttributeDescriptions = attribute_descs.data(),
  };

  vk::PipelineInputAssemblyStateCreateInfo input_assembly{
      .topology = vk::PrimitiveTopology::eTriangleList,
      .primitiveRestartEnable = vk::False,
  };

  vk::PipelineViewportStateCreateInfo viewport{
      .viewportCount = 1,
      .scissorCount = 1,
  };

  vk::PipelineRasterizationStateCreateInfo rasterization{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eClockwise,
      .depthBiasEnable = vk::False,
      .lineWidth = 1.0f,
  };

  vk::PipelineMultisampleStateCreateInfo multisampling{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = vk::False,
  };

  vk::PipelineDepthStencilStateCreateInfo depth_stencil;

  vk::PipelineColorBlendAttachmentState blend_attachment{
      .blendEnable = vk::False,
      .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
  };

  vk::PipelineColorBlendStateCreateInfo color_blending{
      .logicOpEnable = vk::False,
      .attachmentCount = 1,
      .pAttachments = &blend_attachment,
  };

  vk::PushConstantRange push_constants{
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .offset = 0,
      .size = sizeof(glm::mat4) * sizeof(uint32_t),  // scene.cpp DrawParameters
  };

  vk::PipelineLayoutCreateInfo pipeline_layout_info{
      .setLayoutCount = 1,
      .pSetLayouts = &descriptor_set_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_constants,
  };

  pipeline_layout = device.createPipelineLayout(pipeline_layout_info);

  std::vector<Shader> shaders{
      {"triangle.vert", ShaderType::VERTEX},
      {"triangle.frag", ShaderType::FRAGMENT},
  };
  std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
  for (auto& shader : shaders) {
    shader_stages.emplace_back(shader.getStageCreateInfo(device));
  }

  vk::GraphicsPipelineCreateInfo pipeline_info{
      .stageCount = 2,
      .pStages = shader_stages.data(),
      .pVertexInputState = &vertex_input,
      .pInputAssemblyState = &input_assembly,
      .pViewportState = &viewport,
      .pRasterizationState = &rasterization,
      .pMultisampleState = &multisampling,
      .pDepthStencilState = &depth_stencil,
      .pColorBlendState = &color_blending,
      .pDynamicState = &dynamic_state,
      .layout = pipeline_layout,
      .renderPass = render_pass,
      .subpass = 0,
  };

  vk::Result res;  // TODO: check result
  std::tie(res, graphics_pipeline) = device.createGraphicsPipeline(nullptr, pipeline_info);

  for (auto& stage : shader_stages) {
    device.destroyShaderModule(stage.module);
  }
}

void GraphicsContext::createFrameData() {
  auto device = this->device.getDevice();

  max_frames_in_flight = 2;
  frame_data.resize(max_frames_in_flight);

  vk::CommandPoolCreateInfo pool_info{
      .queueFamilyIndex = this->device.getGraphicsQueueIndex(),
  };

  for (auto& frame : frame_data) {
    frame.command_pool = device.createCommandPool(pool_info);
    frame.submit_fence = device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
    frame.acquire_semaphore = device.createSemaphore({});
    frame.release_semaphore = device.createSemaphore({});

    vk::CommandBufferAllocateInfo alloc_info{
        .commandPool = frame.command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };
    frame.command_buffer = device.allocateCommandBuffers(alloc_info)[0];
  }
}

vk::CommandBuffer GraphicsContext::beginTransientExecution() const {
  vk::CommandBufferAllocateInfo allocation_info{
      .commandPool = transient_command_pool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1,
  };

  vk::CommandBuffer cmd = device.getDevice().allocateCommandBuffers(allocation_info)[0];

  vk::CommandBufferBeginInfo begin_info{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  cmd.begin(begin_info);

  return cmd;
}

void GraphicsContext::endTransientExecution(vk::CommandBuffer cmd) const {
  cmd.end();

  vk::SubmitInfo submit_info{
      .commandBufferCount = 1,
      .pCommandBuffers = &cmd,
  };

  device.getQueue().submit(submit_info);
  device.getQueue().waitIdle();
  device.getDevice().freeCommandBuffers(transient_command_pool, cmd);
}
}  // namespace TE
