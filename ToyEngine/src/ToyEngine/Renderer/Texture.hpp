#pragma once

#include <sys/types.h>

#include <cstdint>
#include <vulkan/vulkan.hpp>

#include "ToyEngine/Renderer/Allocator.hpp"

namespace TE {
class Texture {
 public:
  Texture(const std::string& path, uint32_t index);
  ~Texture();

 private:
  VkImage image;
  VmaAllocation allocation;
  vk::ImageView img_view;
  vk::Sampler sampler;

  void transistionImageLayout(vk::CommandBuffer cmd, vk::ImageLayout oldLayout,
                              vk::ImageLayout newLayout);
};
}  // namespace TE