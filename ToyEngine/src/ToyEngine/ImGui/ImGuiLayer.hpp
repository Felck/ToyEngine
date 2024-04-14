#pragma once

#include <vulkan/vulkan.hpp>

#include "ToyEngine/Core/Layer.hpp"
#include "ToyEngine/Core/Timestep.hpp"
#include "ToyEngine/Events/Event.hpp"
#include "ToyEngine/Renderer/GraphicsContext.hpp"

namespace TE {

class ImGuiLayer : public Layer {
 public:
  ImGuiLayer();
  ~ImGuiLayer() = default;

  virtual void onAttach() override;
  virtual void onDetach() override;
  virtual void onUpdate(Timestep dt) override;
  virtual void onEvent(Event& e) override;

 private:
  void createRenderPass(const GraphicsContext& ctx);

  bool block_events = true;
  vk::DescriptorPool descriptor_pool;
  vk::RenderPass render_pass;
};

}  // namespace TE