#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "ToyEngine/Core/Window.hpp"
#include "ToyEngine/Renderer/Shader.hpp"
#include "tepch.hpp"

namespace TE {
class GraphicsContext {
 public:
  GraphicsContext(Window& window);
  ~GraphicsContext();

  void drawFrame();

 private:
  void initVulkan();
  void cleanupVulkan();

  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createDevice();
  void createSwapChain();
  vk::SurfaceFormatKHR selectSurfaceFormat(std::vector<vk::Format> const& preferred_formats);
  void createRenderPass();
  void createGraphicsPipeline();
  void createFramebuffers();
  void createFrameData();
  void recordCommandBuffer(vk::CommandBuffer buffer, uint32_t image_index);

  struct SwapchainData {
    vk::SwapchainKHR swapchain;
    vk::Format format;
    vk::Extent2D extent;
    std::vector<vk::ImageView> image_views;
    std::vector<vk::Framebuffer> framebuffers;
  };

  struct FrameData {
    vk::CommandPool command_pool;
    vk::CommandBuffer command_buffer;
    vk::Fence submit_fence;
    vk::Semaphore acquire_semaphore;
    vk::Semaphore release_semaphore;
  };

  GLFWwindow* window;
  uint32_t max_frames_in_flight;
  uint32_t current_frame = 0;

  vk::Instance instance;
  vk::PhysicalDevice gpu;
  vk::Device device;
  vk::Queue queue;
  vk::SurfaceKHR surface;
  vk::RenderPass render_pass;
  vk::PipelineLayout pipeline_layout;
  vk::Pipeline graphics_pipeline;
  SwapchainData swapchain_data;
  uint32_t graphics_queue_index;
  vk::DebugUtilsMessengerEXT debug_messenger;

  std::vector<FrameData> frame_data;
};
}  // namespace TE