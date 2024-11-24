#pragma once

#include <vulkan/vulkan.hpp>

namespace TE {
vk::ImageView createImageView(vk::Device device, vk::Image image, vk::Format format) {
  vk::ImageViewCreateInfo view_info{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = format,
      .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
  };

  vk::ImageView image_view;
  auto err = device.createImageView(&view_info, nullptr, &image_view);
  if (err != vk::Result::eSuccess) {
    throw std::runtime_error("Failed to create image view!");
  }

  return image_view;
}

}  // namespace TE