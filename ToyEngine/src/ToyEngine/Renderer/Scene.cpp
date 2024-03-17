#include "Scene.hpp"

namespace TE {

void Scene::draw() const {
  auto& ctx = TE::GraphicsContext::get();
  auto cmd = ctx.getCommandBuffer();
  ctx.beginPass();
  cmd.pushConstants(ctx.getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4),
                    &camera.getViewProjection());
  for (auto& vertex_array : vertex_arrays) {
    vertex_array.bind(cmd);
    vertex_array.draw(cmd);
  }
  ctx.endPass();
}
}  // namespace TE