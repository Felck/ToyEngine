#pragma once

#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"
#include "tepch.hpp"

namespace TE {

class VertexArray {
 public:
  typedef float VertexType;
  typedef uint16_t IndexType;

  VertexArray(const std::span<const VertexType> vertices, const std::span<const IndexType> indices);

  void bind(const vk::CommandBuffer cmd) const;
  void draw(const vk::CommandBuffer cmd) const;

 private:
  Buffer vertex_buffer;
  Buffer index_buffer;
  uint32_t index_count;
};
}  // namespace TE