#include "Application.hpp"

#include "ToyEngine/Core/Window.hpp"
#include "ToyEngine/Renderer/GraphicsContext.hpp"

namespace TE {

Application* Application::instance = nullptr;

Application::Application() {
  instance = this;
  window = Window::create();
  window->setEventCallback(BIND_EVENT_FN(onEvent));
}

Application::~Application() {}

void Application::run() {
  GraphicsContext r(*window);

  while (running) {
    for (auto layer : layerStack) {
      layer->onUpdate();
    }
    window->onUpdate();

    r.drawFrame();
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