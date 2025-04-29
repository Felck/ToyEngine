#include <ToyEngine.hpp>
#include <ToyEngine/Core/EntryPoint.hpp>

#include "ToyEngine/Core/Input.hpp"
#include "ToyEngine/Core/KeyCodes.hpp"
#include "ToyEngine/Core/Layer.hpp"
#include "ToyEngine/Core/Timestep.hpp"
#include "ToyEngine/ImGui/ImGuiLayer.hpp"
#include "ToyEngine/Renderer/Scene.hpp"
#include "ToyEngine/Renderer/Texture.hpp"
#include "ToyEngine/Renderer/VertexArray.hpp"

class MainLayer : public TE::Layer {
 public:
  MainLayer() : Layer("Main") {
    textures.emplace_back("assets/textures/Mona_Lisa.png", 0);

    scene.add(
        std::vector<TE::VertexArray::VertexType>{
            {{0.5, -0.5, 0.0}, {1.0, 0.0}},
            {{0.5, 0.5, 0.0}, {1.0, 0.5}},
            {{-0.5, 0.5, 0.0}, {0.0, 0.5}},
            {{-0.5, -0.5, 0.0}, {0.0, 0.0}},
        },
        std::vector<uint16_t>{0, 1, 2, 2, 3, 0});
  }

  void onUpdate(TE::Timestep dt) {
    if (TE::Input::isKeyPressed(TE::Key::Left)) {
      scene.camera.move(-2.0f * dt, 0.0f, 0.0f);
    }
    if (TE::Input::isKeyPressed(TE::Key::Right)) {
      scene.camera.move(2.0 * dt, 0.0f, 0.0f);
    }
    if (TE::Input::isKeyPressed(TE::Key::Up)) {
      scene.camera.move(0.0f, 2.0 * dt, 0.0f);
    }
    if (TE::Input::isKeyPressed(TE::Key::Down)) {
      scene.camera.move(0.0f, -2.0 * dt, 0.0f);
    }

    world = glm::rotate(world, dt * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    scene.ubo.write(&world, sizeof(glm::mat4), 0);
    scene.draw();
  }

 private:
  TE::Scene scene;
  std::vector<TE::Texture> textures;
  glm::mat4 world = glm::mat4(1.0f);
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