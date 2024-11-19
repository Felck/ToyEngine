#include "VertexArray.hpp"

#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"

namespace TE {
VertexArray::VertexArray(const std::span<const VertexType> vertices,
                         const std::span<const IndexType> indices)
    : vertex_buffer(Buffer::createVertexBuffer(sizeof(VertexType) * vertices.size())),
      index_buffer(Buffer::createIndexBuffer(sizeof(IndexType) * indices.size())),
      index_count(indices.size()) {
  vk::DeviceSize vertices_size{sizeof(VertexType) * vertices.size()};
  vk::DeviceSize indices_size{sizeof(IndexType) * indices.size()};

  auto vertex_staging_buffer = Buffer::createStagingBuffer(vertices_size);
  auto index_staging_buffer = Buffer::createStagingBuffer(indices_size);

  vertex_staging_buffer.write(vertices.data(), vertices_size, 0);
  vertex_staging_buffer.copyTo(vertex_buffer);

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