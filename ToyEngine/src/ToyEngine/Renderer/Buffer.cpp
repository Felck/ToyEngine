#include "Buffer.hpp"

#include <vulkan/vulkan.hpp>

#include "Allocator.hpp"
#include "GraphicsContext.hpp"

namespace TE {
Buffer::Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaAllocationCreateFlags flags)
    : size{size} {
  auto& ctx = GraphicsContext::get();
  vk::BufferCreateInfo buffer_info{
      .size = size,
      .usage = usage,
      .sharingMode = vk::SharingMode::eExclusive,
  };

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.flags = flags;

  auto err = vmaCreateBuffer(ctx.getAllocator(), (VkBufferCreateInfo*)&buffer_info, &alloc_info,
                             &buffer, &allocation, nullptr);
  if (err != VK_SUCCESS) {
    throw std::runtime_error("Failed to create buffer");
  }
}

Buffer::~Buffer() { vmaDestroyBuffer(GraphicsContext::get().getAllocator(), buffer, allocation); }

void Buffer::write(const void* data, VkDeviceSize size, VkDeviceSize offset) const {
  auto err = vmaCopyMemoryToAllocation(GraphicsContext::get().getAllocator(), data, allocation,
                                       offset, size);
  if (err != VK_SUCCESS) {
    throw std::runtime_error("Failed to write to buffer");
  }
}

void Buffer::copyTo(Buffer& dst) {
  auto& ctx = GraphicsContext::get();
  ctx.executeTransient([this, &dst](vk::CommandBuffer cmd) {
    vk::BufferCopy copy_region{
        .size = size,
    };

    cmd.copyBuffer(buffer, dst.getBuffer(), 1, &copy_region);
  });
}

}  // namespace TE