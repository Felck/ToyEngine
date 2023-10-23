#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

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
  void createCommandPool();
  void createCommandBuffer();
  void recordCommandBuffer(vk::CommandBuffer buffer, uint32_t image_index);
  void createSyncObjects();

  struct SwapchainData {
    vk::SwapchainKHR swapchain;
    vk::Format format;
    vk::Extent2D extent;
    std::vector<vk::ImageView> image_views;
    std::vector<vk::Framebuffer> framebuffers;
  };

  GLFWwindow* window;
  vk::Instance instance;
  vk::PhysicalDevice gpu;
  vk::Device device;
  vk::Queue queue;
  vk::SurfaceKHR surface;
  vk::RenderPass render_pass;
  vk::PipelineLayout pipeline_layout;
  vk::Pipeline graphics_pipeline;
  vk::CommandPool command_pool;
  vk::CommandBuffer command_buffer;
  SwapchainData swapchain_data;
  uint32_t graphics_queue_index;
  vk::DebugUtilsMessengerEXT debug_messenger;

  vk::Semaphore acquire_semaphore;
  vk::Semaphore render_semaphore;
  vk::Fence submit_fence;
};
}  // namespace TE