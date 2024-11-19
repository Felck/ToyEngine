#include "ToyEngine/Renderer/Allocator.hpp"

namespace TE {
Allocator::Allocator(Device& device) {
  VmaAllocatorCreateInfo allocator_info = {};
  allocator_info.vulkanApiVersion = VK_API_VERSION_1_3;
  allocator_info.physicalDevice = device.getGPU();
  allocator_info.device = device.getDevice();
  allocator_info.instance = device.getInstance();
  vmaCreateAllocator(&allocator_info, &allocator);
}

Allocator::~Allocator() { vmaDestroyAllocator(allocator); }
}  // namespace TE