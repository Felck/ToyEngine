#include <ToyEngine.hpp>
#include <ToyEngine/Core/EntryPoint.hpp>
#include <iostream>

#include "ToyEngine/ImGui/ImGuiLayer.hpp"

class Sandbox : public TE::Application {
 public:
  Sandbox() { pushLayer(new TE::ImGuiLayer()); }

  ~Sandbox() {}
};

TE::Application *TE::createApplication() { return new Sandbox(); }