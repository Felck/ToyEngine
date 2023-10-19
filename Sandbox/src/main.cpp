#include <ToyEngine.hpp>
#include <ToyEngine/Core/EntryPoint.hpp>
#include <iostream>
#include <memory>

#include "ToyEngine/ImGui/ImGuiLayer.hpp"

class Sandbox : public TE::Application {
 public:
  Sandbox() { /*pushLayer(new TE::ImGuiLayer());*/
  }

  ~Sandbox() {}
};

std::unique_ptr<TE::Application> TE::createApplication() { return std::make_unique<Sandbox>(); }