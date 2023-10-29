#include "GraphicsContext.hpp"

#include <vulkan/vulkan.hpp>

#include "ToyEngine/Core/Application.hpp"
#include "ToyEngine/Renderer/Shader.hpp"
#include "tepch.hpp"

// instantiate the default dispatcher
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace {
#ifdef ENABLE_VALIDATION_LAYERS
const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

bool validateLayers() {
  auto available = vk::enumerateInstanceLayerProperties();

  return !std::any_of(
      VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end(), [&available](const auto layer) {
        return !std::any_of(available.begin(), available.end(),
                            [&layer](const auto& lp) { return strcmp(lp.layerName, layer) == 0; });
      });

  auto unavailable = std::find_if(
      VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end(), [&available](const auto layer) {
        return std::find_if(available.begin(), available.end(), [&layer](const auto& l) {
                 return strcmp(l.layerName, layer) == 0;
               }) == available.end();
      });

  return (unavailable == VALIDATION_LAYERS.end());
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData) {
  std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}
#endif

bool validateExtensions(const std::vector<const char*>& required,
                        const std::vector<vk::ExtensionProperties>& available) {
  return !std::any_of(required.begin(), required.end(), [&available](const auto extension) {
    return !std::any_of(available.begin(), available.end(), [&extension](const auto& ep) {
      return strcmp(ep.extensionName, extension) == 0;
    });
  });
}

std::vector<const char*> getRequiredInstanceExtensions() {
  // glfw extensions
  uint32_t glfw_extension_count = 0;
  const char** glfw_extensions;
  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

#ifdef ENABLE_VALIDATION_LAYERS
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

  return extensions;
}
}  // namespace

namespace TE {

GraphicsContext::GraphicsContext(Window& window)
    : window(static_cast<GLFWwindow*>(window.getNativeWindow())) {
  initVulkan();
}

GraphicsContext::~GraphicsContext() {
  device.waitIdle();
  cleanupVulkan();
}

void GraphicsContext::drawFrame() {
  auto& frame = frame_data[current_frame];
  current_frame = (current_frame + 1) % max_frames_in_flight;

  (void)device.waitForFences(frame.submit_fence, true, UINT64_MAX);
  device.resetFences(frame.submit_fence);

  vk::Result res;
  uint32_t image;
  std::tie(res, image) =
      device.acquireNextImageKHR(swapchain_data.swapchain, UINT64_MAX, frame.acquire_semaphore);

  frame.command_buffer.reset();
  recordCommandBuffer(frame.command_buffer, image);

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

  vk::PresentInfoKHR present_info{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signal_semaphores,
      .swapchainCount = 1,
      .pSwapchains = &swapchain_data.swapchain,
      .pImageIndices = &image,
  };
  res = queue.presentKHR(present_info);
}

void GraphicsContext::initVulkan() {
  // initialize function pointers
  VULKAN_HPP_DEFAULT_DISPATCHER.init();
  createInstance();
  createSurface();
  pickPhysicalDevice();
  createDevice();
  createSwapChain();
  createRenderPass();
  createGraphicsPipeline();
  createFramebuffers();
  createFrameData();
}

void TE::GraphicsContext::cleanupVulkan() {
  for (auto& frame : frame_data) {
    device.destroySemaphore(frame.acquire_semaphore);
    device.destroySemaphore(frame.release_semaphore);
    device.destroyFence(frame.submit_fence);
    device.destroyCommandPool(frame.command_pool);
  }

  for (auto framebuffer : swapchain_data.framebuffers) {
    device.destroyFramebuffer(framebuffer);
  }

  if (graphics_pipeline) {
    device.destroyPipeline(graphics_pipeline);
  }

  if (pipeline_layout) {
    device.destroyPipelineLayout(pipeline_layout);
  }

  if (render_pass) {
    device.destroyRenderPass(render_pass);
  }

  for (auto image_view : swapchain_data.image_views) {
    device.destroyImageView(image_view);
  }

  if (swapchain_data.swapchain) {
    device.destroySwapchainKHR(swapchain_data.swapchain);
  }

  if (device) {
    device.destroy();
  }

  if (surface) {
    instance.destroySurfaceKHR(surface);
  }

  if (debug_messenger) {
    instance.destroyDebugUtilsMessengerEXT(debug_messenger);
  }

  instance.destroy();
}

void GraphicsContext::createInstance() {
#ifdef ENABLE_VALIDATION_LAYERS
  if (!validateLayers()) {
    throw std::runtime_error("Validation layers requested, but not available.");
  }
#endif

  // TODO: query application/engine data
  vk::ApplicationInfo application_info{
      .pApplicationName = "VKApp",
      .applicationVersion = 1,
      .pEngineName = "ToyEngine",
      .engineVersion = 1,
      .apiVersion = VK_API_VERSION_1_3,
  };

  auto extensions = getRequiredInstanceExtensions();
  auto available_extensions = vk::enumerateInstanceExtensionProperties();
  if (!validateExtensions(extensions, available_extensions)) {
    throw std::runtime_error("Required instance extensions are missing.");
  }

  vk::InstanceCreateInfo instance_create_info{
      .pApplicationInfo = &application_info,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
  };

#ifdef ENABLE_VALIDATION_LAYERS
  instance_create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
  instance_create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

  vk::DebugUtilsMessengerCreateInfoEXT debug_utils_create_info{
      .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                         vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
      .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                     vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
      .pfnUserCallback = debugCallback,
  };

  instance_create_info.pNext = &debug_utils_create_info;
#else
  instance_create_info.enabledLayerCount = 0;
#endif

  instance = vk::createInstance(instance_create_info);

  // initialize function pointers for instance
  VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

#ifdef ENABLE_VALIDATION_LAYERS
  debug_messenger = instance.createDebugUtilsMessengerEXT(debug_utils_create_info);
#endif
}

void GraphicsContext::createSurface() {
  VkSurfaceKHR s;
  if (glfwCreateWindowSurface(instance, window, nullptr, &s) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface.");
  }
  surface = vk::SurfaceKHR(s);
}

void GraphicsContext::pickPhysicalDevice() {
  std::vector<vk::PhysicalDevice> gpus = instance.enumeratePhysicalDevices();

  bool found_graphics_queue_index = false;
  for (size_t i = 0; i < gpus.size() && !found_graphics_queue_index; i++) {
    gpu = gpus[i];

    auto queue_family_properties = gpu.getQueueFamilyProperties();

    if (queue_family_properties.empty()) {
      throw std::runtime_error("No queue family found.");
    }

    for (uint32_t j = 0; j < static_cast<uint32_t>(queue_family_properties.size()); j++) {
      auto supports_present = gpu.getSurfaceSupportKHR(j, surface);

      if ((queue_family_properties[j].queueFlags & vk::QueueFlagBits::eGraphics) &&
          supports_present) {
        graphics_queue_index = j;
        found_graphics_queue_index = true;
        break;
      }
    }
  }

  if (!found_graphics_queue_index) {
    throw std::runtime_error("Did not find suitable GPU.");
  }
}

void GraphicsContext::createDevice() {
  std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  auto device_extensions = gpu.enumerateDeviceExtensionProperties();
  if (!validateExtensions(extensions, device_extensions)) {
    throw std::runtime_error("Required device extensions are missing.");
  }

  float queue_priority = 1.0f;
  vk::DeviceQueueCreateInfo queue_info{
      .queueFamilyIndex = graphics_queue_index,
      .queueCount = 1,
      .pQueuePriorities = &queue_priority,
  };
  vk::DeviceCreateInfo device_info{
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queue_info,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
  };
  device = gpu.createDevice(device_info);

  // initialize function pointers for device
  VULKAN_HPP_DEFAULT_DISPATCHER.init(device);

  queue = device.getQueue(graphics_queue_index, 0);
}

void GraphicsContext::createSwapChain() {
  auto capabilities = gpu.getSurfaceCapabilitiesKHR(surface);

  // format
  auto format = selectSurfaceFormat(
      {vk::Format::eR8G8B8A8Srgb, vk::Format::eB8G8R8A8Srgb, vk::Format::eA8B8G8R8SrgbPack32});

  // extent
  vk::Extent2D extent;
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    extent = capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    extent.width = std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
    extent.height = std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height);
  }

  // images
  uint32_t image_count = capabilities.minImageCount + 1;
  if ((capabilities.maxImageCount > 0) && (image_count > capabilities.maxImageCount)) {
    image_count = capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR swapchain_create_info{
      .surface = surface,
      .minImageCount = image_count,
      .imageFormat = format.format,
      .imageColorSpace = format.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = capabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = vk::PresentModeKHR::eFifo,
      .clipped = true,
  };
  swapchain_data.swapchain = device.createSwapchainKHR(swapchain_create_info);
  swapchain_data.format = format.format;
  swapchain_data.extent = extent;

  std::vector<vk::Image> images = device.getSwapchainImagesKHR(swapchain_data.swapchain);
  for (const auto& image : images) {
    vk::ImageViewCreateInfo image_view_create_info{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = swapchain_data.format,
        .components =
            {
                vk::ComponentSwizzle::eR,
                vk::ComponentSwizzle::eG,
                vk::ComponentSwizzle::eB,
                vk::ComponentSwizzle::eA,
            },
        .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
    };
    swapchain_data.image_views.push_back(device.createImageView(image_view_create_info));
  }
}

vk::SurfaceFormatKHR GraphicsContext::selectSurfaceFormat(
    const std::vector<vk::Format>& preferred) {
  auto available = gpu.getSurfaceFormatsKHR(surface);

  auto it =
      std::find_if(available.begin(), available.end(), [&preferred](const auto& surface_format) {
        return std::any_of(
            preferred.begin(), preferred.end(),
            [&surface_format](const auto format) { return format == surface_format.format; });
      });

  return it != available.end() ? *it : available[0];
}

void GraphicsContext::createRenderPass() {
  vk::AttachmentDescription color_attachment{
      .format = swapchain_data.format,
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

  render_pass = device.createRenderPass(render_pass_info);
}

void GraphicsContext::createGraphicsPipeline() {
  std::vector<vk::DynamicState> dynamic_states{
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor,
  };

  vk::PipelineDynamicStateCreateInfo dynamic_state{
      .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data(),
  };

  vk::PipelineVertexInputStateCreateInfo vertex_input;

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

  vk::PipelineLayoutCreateInfo pipeline_layout_info{};
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

void GraphicsContext::createFramebuffers() {
  swapchain_data.framebuffers.resize(swapchain_data.image_views.size());

  for (size_t i = 0; i < swapchain_data.image_views.size(); i++) {
    vk::ImageView attachments[] = {swapchain_data.image_views[i]};

    vk::FramebufferCreateInfo framebuffer_info{
        .renderPass = render_pass,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .width = swapchain_data.extent.width,
        .height = swapchain_data.extent.height,
        .layers = 1,
    };

    swapchain_data.framebuffers[i] = device.createFramebuffer(framebuffer_info);
  }
}

void GraphicsContext::createFrameData() {
  max_frames_in_flight = 2;
  frame_data.resize(max_frames_in_flight);

  vk::CommandPoolCreateInfo pool_info{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = graphics_queue_index,
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

void GraphicsContext::recordCommandBuffer(vk::CommandBuffer cmd, uint32_t image_index) {
  vk::ClearValue clear_value{{{{0.01f, 0.01f, 0.033f, 1.0f}}}};

  vk::Viewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(swapchain_data.extent.width),
      .height = static_cast<float>(swapchain_data.extent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };

  vk::Rect2D scissor{{0, 0}, swapchain_data.extent};

  vk::CommandBufferBeginInfo begin_info{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

  vk::RenderPassBeginInfo render_pass_info{
      .renderPass = render_pass,
      .framebuffer = swapchain_data.framebuffers[image_index],
      .renderArea = {{0, 0}, swapchain_data.extent},
      .clearValueCount = 1,
      .pClearValues = &clear_value,
  };

  cmd.begin(begin_info);
  cmd.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipeline);
  cmd.setViewport(0, viewport);
  cmd.setScissor(0, scissor);
  cmd.draw(3, 1, 0, 0);
  cmd.endRenderPass();
  cmd.end();
}
}  // namespace TE
