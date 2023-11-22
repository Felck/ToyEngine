#include "Buffer.hpp"

#include <vulkan/vulkan.hpp>

#include "GraphicsContext.hpp"
#include "tepch.hpp"

namespace TE {
Buffer::Buffer(const GraphicsContext& ctx, vk::DeviceSize size, vk::BufferUsageFlags usage,
               vk::MemoryPropertyFlags properties)
    : ctx{ctx}, size{size} {
  vk::BufferCreateInfo buffer_info{
      .size = size,
      .usage = usage,
      .sharingMode = vk::SharingMode::eExclusive,
  };
  buffer = ctx.getDevice().createBuffer(buffer_info);
  vk::MemoryRequirements mem_requirements = ctx.getDevice().getBufferMemoryRequirements(buffer);

  vk::MemoryAllocateInfo alloc_info{
      .allocationSize = mem_requirements.size,
      .memoryTypeIndex = findMemoryType(ctx.getGPU(), mem_requirements.memoryTypeBits, properties),
  };
  memory = ctx.getDevice().allocateMemory(alloc_info);
  ctx.getDevice().bindBufferMemory(buffer, memory, 0);
}

Buffer::~Buffer() {
  ctx.getDevice().destroyBuffer(buffer);
  ctx.getDevice().freeMemory(memory);
}

void Buffer::write(const void* data, VkDeviceSize size, VkDeviceSize offset) const {
  void* mapped = ctx.getDevice().mapMemory(memory, offset, size);
  memcpy(mapped, data, (size_t)size);
  ctx.getDevice().unmapMemory(memory);
}

void Buffer::copyTo(Buffer& dst) {
  vk::CommandBufferAllocateInfo alloc_info{
      .commandPool = ctx.getCommandPool(),
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1,
  };
  vk::CommandBuffer cmd = ctx.getDevice().allocateCommandBuffers(alloc_info)[0];
  vk::CommandBufferBeginInfo begin_info{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  vk::BufferCopy copy_region{
      .srcOffset = 0,
      .dstOffset = 0,
      .size = size,
  };

  cmd.begin(begin_info);
  cmd.copyBuffer(buffer, dst.getBuffer(), 1, &copy_region);
  cmd.end();
  vk::SubmitInfo submit_info{
      .commandBufferCount = 1,
      .pCommandBuffers = &cmd,
  };

  ctx.getQueue().submit(submit_info);
  ctx.getQueue().waitIdle();
  ctx.getDevice().freeCommandBuffers(ctx.getCommandPool(), cmd);
}

uint32_t Buffer::findMemoryType(vk::PhysicalDevice gpu, uint32_t type_filter,
                                vk::MemoryPropertyFlags properties) const {
  vk::PhysicalDeviceMemoryProperties mem_properties = gpu.getMemoryProperties();
  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    if ((type_filter & (1 << i)) &&
        (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("No suitable memory type.");
}
}  // namespace TE