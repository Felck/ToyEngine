#include <ToyEngine.hpp>
#include <ToyEngine/Core/EntryPoint.hpp>

#include "ToyEngine/Core/Input.hpp"
#include "ToyEngine/Core/KeyCodes.hpp"
#include "ToyEngine/Core/Layer.hpp"
#include "ToyEngine/ImGui/ImGuiLayer.hpp"
#include "ToyEngine/Renderer/Scene.hpp"

class MainLayer : public TE::Layer {
 public:
  MainLayer() : Layer("Main") {
    scene.add(std::vector<float>{0.0, -0.5, 0.5, 0.5, -0.5, 0.5}, std::vector<uint16_t>{0, 1, 2});
  }
  void onUpdate() {
    if (TE::Input::isKeyPressed(TE::Key::Left)) {
      scene.camera.move(-0.01f, 0.0f, 0.0f);
    }
    if (TE::Input::isKeyPressed(TE::Key::Right)) {
      scene.camera.move(0.01f, 0.0f, 0.0f);
    }
    if (TE::Input::isKeyPressed(TE::Key::Up)) {
      scene.camera.move(0.0f, 0.01f, 0.0f);
    }
    if (TE::Input::isKeyPressed(TE::Key::Down)) {
      scene.camera.move(0.0f, -0.01f, 0.0f);
    }
    scene.draw();
  }

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