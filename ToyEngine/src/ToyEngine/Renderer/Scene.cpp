#include "Scene.hpp"

namespace TE {

void Scene::draw() const {
  auto& ctx = TE::GraphicsContext::get();
  auto cmd = ctx.getCommandBuffer();
  ctx.beginPass();
  for (auto& vertex_array : vertex_arrays) {
    vertex_array.bind(cmd);
    vertex_array.draw(cmd);
  }
  ctx.endPass();
}
}  // namespace TE