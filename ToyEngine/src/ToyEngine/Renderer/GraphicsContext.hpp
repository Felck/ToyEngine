#include <vulkan/vulkan.hpp>

#include "ToyEngine/Core/Window.hpp"
#include "tepch.hpp"

namespace TE {
class GraphicsContext {
 public:
  GraphicsContext(Window& window);
  ~GraphicsContext();

 private:
  void initVulkan();
  void cleanupVulkan();

  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createDevice();
  void createSwapChain();
  void createGraphicsPipeline();
  vk::SurfaceFormatKHR selectSurfaceFormat(std::vector<vk::Format> const& preferred_formats);

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
  SwapchainData swapchain_data;
  uint32_t graphics_queue_index;
  vk::DebugUtilsMessengerEXT debug_messenger;
};
}  // namespace TE