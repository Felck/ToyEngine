#include "Application.hpp"

#include <GLFW/glfw3.h>

namespace TE {

Application::Application() {
  window = std::unique_ptr<Window>(Window::create());
}

Application::~Application() {}

void Application::run() {
  while (running) {
    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    window->onUpdate();
  }
}

}  // namespace TE