#pragma once

#include "Layer.hpp"
#include "LayerStack.hpp"
#include "ToyEngine/Events/Event.hpp"
#include "ToyEngine/Events/WindowEvent.hpp"
#include "Window.hpp"
#include "tepch.hpp"

namespace TE {

class Application {
 public:
  Application();
  virtual ~Application();

  void run();
  void onEvent(Event& e);
  void pushLayer(Layer* layer);
  void pushOverlay(Layer* layer);

 private:
  bool onWindowClose(WindowCloseEvent& e);

  std::unique_ptr<Window> window;
  LayerStack layerStack;
  bool running = true;
};

// To be defined in client
Application* createApplication();
}  // namespace TE