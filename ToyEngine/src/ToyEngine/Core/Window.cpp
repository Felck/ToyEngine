#include "Window.hpp"

namespace TE {

Window* Window::create(const WindowProps& props) { return new Window(props); }

Window::Window(const WindowProps& props) { init(props); }

Window::~Window() { shutdown(); }

void Window::init(const WindowProps& props) {
  data.Title = props.Title;
  data.Width = props.Width;
  data.Height = props.Height;

  // TODO: assert
  // TODO: glfwTerminate on system shutdown
  glfwInit();

  window = glfwCreateWindow((int)props.Width, (int)props.Height,
                            data.Title.c_str(), nullptr, nullptr);
  glfwMakeContextCurrent(window);
  glfwSetWindowUserPointer(window, &data);
  setVSync(true);
}

void Window::shutdown() { glfwDestroyWindow(window); }

void Window::onUpdate() {
  glfwPollEvents();
  glfwSwapBuffers(window);
}

void Window::setVSync(bool enabled) {
  if (enabled) {
    glfwSwapInterval(1);
  } else {
    glfwSwapInterval(0);
  }

  data.VSync = enabled;
}

bool Window::isVSync() const { return data.VSync; }

}  // namespace TE