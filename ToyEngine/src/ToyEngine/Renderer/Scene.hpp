#pragma once

#include "ToyEngine/Renderer/Camera.hpp"
#include "ToyEngine/Renderer/GraphicsContext.hpp"
#include "ToyEngine/Renderer/VertexArray.hpp"
#include "tepch.hpp"

namespace TE {
class Scene {
 public:
  void draw();
  inline void add(const std::span<const VertexArray::VertexType> vertices,
                  const std::span<const VertexArray::IndexType> indices) {
    vertex_arrays.emplace_back(vertices, indices);
  }

  Camera camera{glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f)};

 private:
  std::vector<VertexArray> vertex_arrays;
};
}  // namespace TE