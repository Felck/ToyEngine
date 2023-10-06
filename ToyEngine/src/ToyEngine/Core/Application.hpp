#pragma once

#include "Window.hpp"
#include "tepch.hpp"

namespace TE {

class Application {
 public:
  Application();
  virtual ~Application();

  void run();

 private:
  std::unique_ptr<Window> window;
  bool running = true;
};

// To be defined in client
Application *createApplication();
}  // namespace TE