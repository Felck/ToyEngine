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
  vk::SurfaceFormatKHR selectSurfaceFormat(std::vector<vk::Format> const& preferred_formats);

  struct SwapchainData {
    vk::SwapchainKHR swapchain;
    std::vector<vk::Image> swap_chain_images;
    vk::Format format;
    vk::Extent2D extent;
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