#pragma once

#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"
#include "tepch.hpp"

namespace TE {
class GraphicsContext;  // forward declaration

class VertexArray {
 public:
  typedef float VertexType;
  typedef uint16_t IndexType;

  VertexArray(GraphicsContext& ctx, std::vector<VertexType> vertices,
              std::vector<IndexType> indices);

  void bind(const vk::CommandBuffer& cmd) const;
  void draw(const vk::CommandBuffer& cmd) const;

 private:
  Buffer vertex_buffer;
  Buffer index_buffer;
  uint32_t index_count;
};
}  // namespace TE