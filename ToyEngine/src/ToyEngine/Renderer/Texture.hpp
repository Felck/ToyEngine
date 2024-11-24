#pragma once

#include <vulkan/vulkan.hpp>

#include "ToyEngine/Renderer/Allocator.hpp"

namespace TE {
class Texture {
 public:
  Texture(const std::string& path);
  ~Texture();

  void transistionImageLayout(vk::CommandBuffer cmd, vk::ImageLayout oldLayout,
                              vk::ImageLayout newLayout);

 private:
  VkImage image;
  VmaAllocation allocation;
  vk::ImageView img_view;
};
}  // namespace TE