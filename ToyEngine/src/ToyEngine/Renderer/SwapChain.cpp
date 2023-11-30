#include "SwapChain.hpp"

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include "ToyEngine/Renderer/Device.hpp"
#include "tepch.hpp"

namespace TE {

SwapChain::SwapChain(GLFWwindow* window, const Device& device) : window{window}, device{device} {
  init();
}

SwapChain::~SwapChain() { destroy(); }

void SwapChain::init() {
  auto capabilities = device.getGPU().getSurfaceCapabilitiesKHR(device.getSurface());

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
      .surface = device.getSurface(),
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
  this->swapchain = device.getDevice().createSwapchainKHR(swapchain_create_info);
  this->format = format.format;
  this->extent = extent;

  std::vector<vk::Image> images = device.getDevice().getSwapchainImagesKHR(this->swapchain);
  for (const auto& image : images) {
    vk::ImageViewCreateInfo image_view_create_info{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = this->format,
        .components =
            {
                vk::ComponentSwizzle::eR,
                vk::ComponentSwizzle::eG,
                vk::ComponentSwizzle::eB,
                vk::ComponentSwizzle::eA,
            },
        .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
    };
    this->image_views.push_back(device.getDevice().createImageView(image_view_create_info));
  }
}

void SwapChain::resize() {
  device.getDevice().waitIdle();

  destroy();
  init();
  createFramebuffers(render_pass);
}

void SwapChain::destroy() {
  destroyFramebuffers();

  for (auto image_view : this->image_views) {
    device.getDevice().destroyImageView(image_view);
  }
  image_views.clear();

  if (this->swapchain) {
    device.getDevice().destroySwapchainKHR(this->swapchain);
  }
}

void SwapChain::createFramebuffers(vk::RenderPass render_pass) {
  this->render_pass = render_pass;
  this->framebuffers.resize(this->image_views.size());

  for (size_t i = 0; i < this->image_views.size(); i++) {
    vk::ImageView attachments[] = {this->image_views[i]};

    vk::FramebufferCreateInfo framebuffer_info{
        .renderPass = render_pass,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .width = this->extent.width,
        .height = this->extent.height,
        .layers = 1,
    };

    this->framebuffers[i] = device.getDevice().createFramebuffer(framebuffer_info);
  }
}

void SwapChain::destroyFramebuffers() {
  for (auto framebuffer : this->framebuffers) {
    device.getDevice().destroyFramebuffer(framebuffer);
  }
  framebuffers.clear();
}

vk::SurfaceFormatKHR SwapChain::selectSurfaceFormat(const std::vector<vk::Format>& preferred) {
  auto available = device.getGPU().getSurfaceFormatsKHR(device.getSurface());

  auto it =
      std::find_if(available.begin(), available.end(), [&preferred](const auto& surface_format) {
        return std::any_of(
            preferred.begin(), preferred.end(),
            [&surface_format](const auto format) { return format == surface_format.format; });
      });

  return it != available.end() ? *it : available[0];
}

}  // namespace TE
