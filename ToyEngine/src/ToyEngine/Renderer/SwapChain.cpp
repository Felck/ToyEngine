#include "SwapChain.hpp"

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include "ToyEngine/Renderer/Device.hpp"
#include "ToyEngine/Renderer/Helpers.hpp"
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
    this->image_views.push_back(createImageView(device.getDevice(), image, this->format));
    this->submit_semaphores.push_back(device.getDevice().createSemaphore({}));
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

  for (auto semaphore : this->submit_semaphores) {
    device.getDevice().destroySemaphore(semaphore);
  }
  submit_semaphores.clear();

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

void SwapChain::acquireNextImage(vk::Semaphore acquire_semaphore) {
  vk::Result res;
  std::tie(res, current_image) =
      device.getDevice().acquireNextImageKHR(swapchain, UINT64_MAX, acquire_semaphore);
  if (res == vk::Result::eErrorOutOfDateKHR) {
    resize();
    acquireNextImage(acquire_semaphore);
    return;
  } else if (res == vk::Result::eSuboptimalKHR) {
    vk::PipelineStageFlags psf[] = {vk::PipelineStageFlagBits::eBottomOfPipe};
    vk::SubmitInfo submit_info{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &acquire_semaphore,
        .pWaitDstStageMask = psf,
    };
    // clear signaled semaphore
    device.getQueue().submit(submit_info);

    resize();
    acquireNextImage(acquire_semaphore);
    return;
  } else if (res != vk::Result::eSuccess) {
    device.getQueue().waitIdle();
    throw std::runtime_error("Failed to acquire swap chain image!");
  }
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
