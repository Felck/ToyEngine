#pragma once

#include "ToyEngine/Core/Layer.hpp"
#include "ToyEngine/Events/Event.hpp"

namespace TE {

class ImGuiLayer : public Layer {
 public:
  ImGuiLayer();
  ~ImGuiLayer() = default;

  virtual void onAttach() override;
  virtual void onDetach() override;
  virtual void onUpdate() override;
  virtual void onEvent(Event& e) override;

  void begin();
  void end();

 private:
  bool m_BlockEvents = true;
};

}  // namespace TE