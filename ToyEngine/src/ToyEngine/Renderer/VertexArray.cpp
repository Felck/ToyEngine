#include "VertexArray.hpp"

#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"

namespace TE {
VertexArray::VertexArray(const std::span<const VertexType> vertices,
                         const std::span<const IndexType> indices)
    : vertex_buffer{
          sizeof(VertexType) * vertices.size(),
          vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
          vk::MemoryPropertyFlagBits::eDeviceLocal,
      },
      index_buffer{
          sizeof(IndexType) * indices.size(),
          vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
          vk::MemoryPropertyFlagBits::eDeviceLocal,
      },
      index_count(indices.size()) {
  vk::DeviceSize vertices_size{sizeof(VertexType) * vertices.size()};
  vk::DeviceSize indices_size{sizeof(IndexType) * indices.size()};

  Buffer vertex_staging_buffer{
      vertices_size,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
  };
  vertex_staging_buffer.write(vertices.data(), vertices_size, 0);
  vertex_staging_buffer.copyTo(vertex_buffer);

  Buffer index_staging_buffer{
      indices_size,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
  };
  index_staging_buffer.write(indices.data(), indices_size, 0);
  index_staging_buffer.copyTo(index_buffer);
}

void VertexArray::bind(const vk::CommandBuffer cmd) const {
  cmd.bindVertexBuffers(0, vertex_buffer.getBuffer(), {0});
  cmd.bindIndexBuffer(index_buffer.getBuffer(), 0, vk::IndexType::eUint16);
}

void VertexArray::draw(const vk::CommandBuffer cmd) const {
  cmd.drawIndexed(index_count, 1, 0, 0, 0);
}
}  // namespace TE