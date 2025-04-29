#include "Scene.hpp"

#include "ToyEngine/Renderer/Buffer.hpp"
#include "ToyEngine/Renderer/GraphicsContext.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace TE {

struct DrawParameters {
  glm::mat4 viewProjection;
  uint32_t world;
};

Scene::Scene() : ubo{Buffer::createUniformBuffer(sizeof(glm::mat4))} {
  auto& ctx = TE::GraphicsContext::get();

  glm::mat4 world = glm::translate(glm::mat4(1.0f), glm::vec3(0.1f, 0.2f, 0.0f));
  ubo.write(&world, sizeof(glm::mat4), 0);

  vk::DescriptorBufferInfo buffer_info{
      .buffer = ubo.getBuffer(),
      .offset = 0,
      .range = sizeof(glm::mat4),
  };
  vk::WriteDescriptorSet write_set{
      .dstSet = ctx.getDescriptorSet(),
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = vk::DescriptorType::eUniformBuffer,
      .pBufferInfo = &buffer_info,
  };
  ctx.getDevice().updateDescriptorSets(write_set, nullptr);
}

void Scene::draw() {
  auto& ctx = TE::GraphicsContext::get();
  auto cmd = ctx.getCommandBuffer();
  ctx.beginPass();
  cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, ctx.getPipelineLayout(), 0,
                         ctx.getDescriptorSet(), nullptr);

  DrawParameters draw_parameters{
      .viewProjection = camera.getViewProjection(),
      .world = 0,
  };
  cmd.pushConstants(ctx.getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0,
                    sizeof(DrawParameters), &draw_parameters);

  for (auto& vertex_array : vertex_arrays) {
    vertex_array.bind(cmd);
    vertex_array.draw(cmd);
  }
  ctx.endPass();
}
}  // namespace TE