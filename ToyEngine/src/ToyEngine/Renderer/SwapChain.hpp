#pragma once

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include "ToyEngine/Renderer/Device.hpp"
#include "tepch.hpp"

namespace TE {
class SwapChain {
 public:
  SwapChain(GLFWwindow* window, Device& device);
  ~SwapChain();

  void resize();
  void createFramebuffers(vk::RenderPass render_pass);
  void destroyFramebuffers();

  inline vk::SwapchainKHR& get() { return this->swapchain; }
  inline vk::Format getFormat() { return this->format; }
  inline vk::Extent2D getExtent() { return this->extent; }
  inline vk::Framebuffer& getFramebuffer(uint32_t index) { return this->framebuffers[index]; }

 private:
  void init();
  void destroy();
  vk::SurfaceFormatKHR selectSurfaceFormat(const std::vector<vk::Format>& preferred);

  GLFWwindow* window;
  Device& device;
  vk::RenderPass render_pass;
  vk::SwapchainKHR swapchain;
  vk::Format format;
  vk::Extent2D extent;
  std::vector<vk::ImageView> image_views;
  std::vector<vk::Framebuffer> framebuffers;
};

}  // namespace TE