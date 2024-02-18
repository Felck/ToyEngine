#pragma once

#include "ToyEngine/Renderer/GraphicsContext.hpp"
#include "ToyEngine/Renderer/VertexArray.hpp"
#include "tepch.hpp"

namespace TE {
class Scene {
 public:
  void draw() const;
  inline void add(const std::span<const VertexArray::VertexType> vertices,
                  const std::span<const VertexArray::IndexType> indices) {
    vertex_arrays.emplace_back(vertices, indices);
  }

 private:
  std::vector<VertexArray> vertex_arrays;
};
}  // namespace TE