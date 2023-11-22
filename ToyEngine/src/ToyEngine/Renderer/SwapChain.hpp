#pragma once

#include <vulkan/vulkan.hpp>

#include "tepch.hpp"

namespace TE {
class GraphicsContext;  // forward declaration

class SwapChain {
 public:
  SwapChain(GraphicsContext& ctx);

  void init();
  void resize();
  void destroy();
  void createFramebuffers(vk::RenderPass render_pass);
  void destroyFramebuffers();

  inline vk::SwapchainKHR& get() { return this->swapchain; }
  inline vk::Format getFormat() { return this->format; }
  inline vk::Extent2D getExtent() { return this->extent; }
  inline vk::Framebuffer& getFramebuffer(uint32_t index) { return this->framebuffers[index]; }

 private:
  vk::SurfaceFormatKHR selectSurfaceFormat(const std::vector<vk::Format>& preferred);

  GraphicsContext& ctx;
  vk::RenderPass render_pass;
  vk::SwapchainKHR swapchain;
  vk::Format format;
  vk::Extent2D extent;
  std::vector<vk::ImageView> image_views;
  std::vector<vk::Framebuffer> framebuffers;
};

}  // namespace TE