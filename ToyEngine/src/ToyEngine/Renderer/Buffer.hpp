#pragma once

#include <vulkan/vulkan.hpp>

#include "ToyEngine/Renderer/Allocator.hpp"

namespace TE {
class Buffer {
 public:
  Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaAllocationCreateFlags flags);
  ~Buffer();

  void write(const void* data, VkDeviceSize size, VkDeviceSize offset) const;
  void copyTo(Buffer& dst);

  inline vk::Buffer getBuffer() const { return buffer; };
  inline vk::DeviceSize getSize() const { return size; };

  inline static Buffer createVertexBuffer(vk::DeviceSize size) {
    return Buffer{
        size,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        0,
    };
  }

  inline static Buffer createIndexBuffer(vk::DeviceSize size) {
    return Buffer{
        size,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        0,
    };
  }

  inline static Buffer createUniformBuffer(vk::DeviceSize size) {
    return Buffer{
        size,
        vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
    };
  }

  inline static Buffer createStagingBuffer(vk::DeviceSize size) {
    return Buffer{
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
    };
  }

 private:
  uint32_t findMemoryType(vk::PhysicalDevice gpu, uint32_t type_filter,
                          vk::MemoryPropertyFlags properties) const;

  VkBuffer buffer;
  VmaAllocation allocation;
  vk::DeviceSize size;
};
}  // namespace TE