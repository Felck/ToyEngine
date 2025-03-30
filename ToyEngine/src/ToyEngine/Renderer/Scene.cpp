#include "Scene.hpp"

#include "ToyEngine/Renderer/GraphicsContext.hpp"

namespace TE {

void Scene::draw() {
  auto& ctx = TE::GraphicsContext::get();
  auto cmd = ctx.getCommandBuffer();
  ctx.beginPass();
  cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, ctx.getPipelineLayout(), 0,
                         ctx.getDescriptorSet(), nullptr);
  cmd.pushConstants(ctx.getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4),
                    &camera.getViewProjection());
  for (auto& vertex_array : vertex_arrays) {
    vertex_array.bind(cmd);
    vertex_array.draw(cmd);
  }
  ctx.endPass();
}
}  // namespace TE