#pragma once

#include <vulkan/vulkan.hpp>

#include "tepch.hpp"

namespace TE {
class GraphicsContext;  // forward declaration

class Buffer {
 public:
  Buffer(const GraphicsContext& ctx, vk::DeviceSize size, vk::BufferUsageFlags usage,
         vk::MemoryPropertyFlags properties);
  ~Buffer();

  void write(const void* data, VkDeviceSize size, VkDeviceSize offset) const;
  void copyTo(Buffer& dst);

  inline vk::Buffer getBuffer() const { return buffer; };
  inline vk::DeviceSize getSize() const { return size; };

 private:
  uint32_t findMemoryType(vk::PhysicalDevice gpu, uint32_t type_filter,
                          vk::MemoryPropertyFlags properties) const;

  const GraphicsContext& ctx;
  vk::Buffer buffer;
  vk::DeviceMemory memory;
  vk::DeviceSize size;
};
}  // namespace TE