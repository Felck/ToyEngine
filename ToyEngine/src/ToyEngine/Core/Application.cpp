#include "Application.hpp"

#include "ToyEngine/Core/Input.hpp"
#include "ToyEngine/Core/Window.hpp"
#include "ToyEngine/Renderer/GraphicsContext.hpp"

namespace TE {

Application* Application::instance = nullptr;

Application::Application() {
  instance = this;
  window = Window::create();
  window->setEventCallback(BIND_EVENT_FN(onEvent));
  Input::init(window->getNativeWindow());
}

Application::~Application() { GraphicsContext::get().getDevice().waitIdle(); }

void Application::run() {
  while (running) {
    window->onUpdate();

    GraphicsContext::get().beginFrame();
    for (auto layer : layerStack) {
      layer->onUpdate();
    }
    GraphicsContext::get().endFrame();
  }
}

void Application::onEvent(Event& e) {
  e.dispatch<WindowCloseEvent>(BIND_EVENT_FN(onWindowClose));

  for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it) {
    if (e.handled) {
      break;
    }
    (*it)->onEvent(e);
  }
}

void Application::pushLayer(Layer* layer) { layerStack.pushLayer(layer); }

void Application::pushOverlay(Layer* layer) { layerStack.pushOverlay(layer); }

bool Application::onWindowClose([[maybe_unused]] WindowCloseEvent& e) {
  running = false;
  return true;
}

}  // namespace TE