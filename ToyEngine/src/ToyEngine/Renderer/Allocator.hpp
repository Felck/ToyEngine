#pragma once

#include <vk_mem_alloc.h>

#include "ToyEngine/Renderer/Device.hpp"

namespace TE {
class Allocator {
 public:
  Allocator(Device& device);
  ~Allocator();

 private:
  VmaAllocator allocator;
};
}  // namespace TE