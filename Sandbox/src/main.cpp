#include <ToyEngine.hpp>
#include <ToyEngine/Core/EntryPoint.hpp>

#include "ToyEngine/Core/Layer.hpp"
#include "ToyEngine/ImGui/ImGuiLayer.hpp"
#include "ToyEngine/Renderer/Scene.hpp"

class MainLayer : public TE::Layer {
 public:
  MainLayer() : Layer("Main") {
    scene.add(std::vector<float>{0.0, -0.5, 0.5, 0.5, -0.5, 0.5}, std::vector<uint16_t>{0, 1, 2});
  }
  void onUpdate() { scene.draw(); }

 private:
  TE::Scene scene;
};

class Sandbox : public TE::Application {
 public:
  Sandbox() {
    pushLayer(new MainLayer());
    pushLayer(new TE::ImGuiLayer());
  }

  ~Sandbox() {}
};

std::unique_ptr<TE::Application> TE::createApplication() { return std::make_unique<Sandbox>(); }