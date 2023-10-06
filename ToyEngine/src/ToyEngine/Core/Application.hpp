#pragma once

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

 private:
  bool onWindowClose(WindowCloseEvent& e);

  std::unique_ptr<Window> window;
  bool running = true;
};

// To be defined in client
Application* createApplication();
}  // namespace TE