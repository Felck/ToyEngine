#include "ToyEngine/ImGui/ImGuiLayer.hpp"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <vulkan/vulkan.hpp>

#include "ToyEngine/Core/Application.hpp"
#include "ToyEngine/Core/Layer.hpp"
#include "ToyEngine/Events/Event.hpp"
#include "ToyEngine/Renderer/GraphicsContext.hpp"
#include "tepch.hpp"

namespace TE {

ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

void ImGuiLayer::onAttach() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();

  Application& app = Application::get();
  GLFWwindow* window = static_cast<GLFWwindow*>(app.getWindow().getNativeWindow());
  GraphicsContext& ctx = GraphicsContext::get();

  vk::DescriptorPoolSize pool_sizes[] = {
      {vk::DescriptorType::eSampler, 1000},
      {vk::DescriptorType::eCombinedImageSampler, 1000},
      {vk::DescriptorType::eSampledImage, 1000},
      {vk::DescriptorType::eStorageImage, 1000},
      {vk::DescriptorType::eUniformTexelBuffer, 1000},
      {vk::DescriptorType::eStorageTexelBuffer, 1000},
      {vk::DescriptorType::eUniformBuffer, 1000},
      {vk::DescriptorType::eStorageBuffer, 1000},
      {vk::DescriptorType::eUniformBufferDynamic, 1000},
      {vk::DescriptorType::eStorageBufferDynamic, 1000},
      {vk::DescriptorType::eInputAttachment, 1000},
  };
  vk::DescriptorPoolCreateInfo pool_info = {
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = 1000 * IM_ARRAYSIZE(pool_sizes),
      .poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes),
      .pPoolSizes = pool_sizes,
  };
  descriptor_pool = ctx.getDevice().createDescriptorPool(pool_info);
  createRenderPass(ctx);

  ImGui_ImplGlfw_InitForVulkan(window, true);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = ctx.getInstance();
  init_info.PhysicalDevice = ctx.getGPU();
  init_info.Device = ctx.getDevice();
  init_info.QueueFamily = ctx.getGraphicsQueueIndex();
  init_info.Queue = ctx.getQueue();
  // init_info.PipelineCache = g_PipelineCache;
  init_info.DescriptorPool = descriptor_pool;
  init_info.Subpass = 0;
  init_info.MinImageCount = ctx.getSwapChain().getImageCount();
  init_info.ImageCount = ctx.getSwapChain().getImageCount();
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  // init_info.Allocator = g_Allocator;
  // init_info.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&init_info, render_pass);

  ctx.executeTransient([&](VkCommandBuffer cmd) { ImGui_ImplVulkan_CreateFontsTexture(cmd); });

  // clear font textures from cpu data
  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGuiLayer::onDetach() {
  const auto& device = GraphicsContext::get().getDevice();
  device.waitIdle();

  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  device.destroyDescriptorPool(descriptor_pool);
  device.destroyRenderPass(render_pass);
}

void ImGuiLayer::onUpdate() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::ShowDemoWindow();

  ImGui::Render();
  ImDrawData* draw_data = ImGui::GetDrawData();

  GraphicsContext::get().record([=](auto cmd) {
    auto& swapchain = GraphicsContext::get().getSwapChain();
    vk::ClearValue clear_value{{{{0.01f, 0.01f, 0.033f, 1.0f}}}};

    vk::RenderPassBeginInfo render_pass_info{
        .renderPass = render_pass,
        .framebuffer = swapchain.getFramebuffer(),
        .renderArea = {{0, 0}, swapchain.getExtent()},
        .clearValueCount = 1,
        .pClearValues = &clear_value,
    };
    cmd.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    ImGui_ImplVulkan_RenderDrawData(draw_data, cmd);
    cmd.endRenderPass();
  });
}

void ImGuiLayer::onEvent(Event& e) {
  if (block_events) {
    ImGuiIO& io = ImGui::GetIO();
    e.handled |= e.isInCategory(MouseCategory) & io.WantCaptureMouse;
    e.handled |= e.isInCategory(KeyboardCategory) & io.WantCaptureKeyboard;
  }
}

void ImGuiLayer::createRenderPass(const GraphicsContext& ctx) {
  vk::AttachmentDescription color_attachment{
      .format = ctx.getSwapChain().getFormat(),
      .samples = vk::SampleCountFlagBits::e1,
      .loadOp = vk::AttachmentLoadOp::eDontCare,
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

  render_pass = ctx.getDevice().createRenderPass(render_pass_info);
}
}  // namespace TE