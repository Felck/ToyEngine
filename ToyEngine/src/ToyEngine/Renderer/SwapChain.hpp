#pragma once

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include "ToyEngine/Renderer/Device.hpp"

namespace TE {
class SwapChain {
 public:
  SwapChain(GLFWwindow* window, const Device& device);
  ~SwapChain();

  void resize();
  void createFramebuffers(vk::RenderPass render_pass);
  void destroyFramebuffers();
  void acquireNextImage(vk::Semaphore acquire_semaphore);

  inline vk::SwapchainKHR get() const { return swapchain; }
  inline vk::Format getFormat() const { return format; }
  inline vk::Extent2D getExtent() const { return extent; }
  inline uint32_t getImageCount() const { return image_views.size(); }
  inline uint32_t getImage() const { return current_image; }
  inline vk::Framebuffer getFramebuffer() const { return framebuffers[current_image]; }

 private:
  void init();
  void destroy();
  vk::SurfaceFormatKHR selectSurfaceFormat(const std::vector<vk::Format>& preferred);

  GLFWwindow* window;
  const Device& device;
  vk::RenderPass render_pass;
  vk::SwapchainKHR swapchain;
  vk::Format format;
  vk::Extent2D extent;
  std::vector<vk::ImageView> image_views;
  std::vector<vk::Framebuffer> framebuffers;
  uint32_t current_image;
};

}  // namespace TE