#pragma once

#include <vk_mem_alloc.h>

#include "ToyEngine/Renderer/Device.hpp"

namespace TE {
class Allocator {
 public:
  Allocator(Device& device);
  ~Allocator();

  VmaAllocator getAllocator() const { return allocator; }

 private:
  VmaAllocator allocator;
};
}  // namespace TE