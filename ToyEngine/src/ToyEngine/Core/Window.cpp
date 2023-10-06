#include "Window.hpp"

#include "ToyEngine/Events/KeyEvent.hpp"
#include "ToyEngine/Events/MouseEvent.hpp"
#include "ToyEngine/Events/WindowEvent.hpp"

namespace TE {

Window* Window::create(const WindowProps& props) { return new Window(props); }

Window::Window(const WindowProps& props) { init(props); }

Window::~Window() { shutdown(); }

void Window::init(const WindowProps& props) {
  data.title = props.Title;
  data.width = props.Width;
  data.height = props.Height;

  // TODO: assert
  // TODO: error callback
  // TODO: glfwTerminate on system shutdown
  glfwInit();

  window = glfwCreateWindow((int)props.Width, (int)props.Height,
                            data.title.c_str(), nullptr, nullptr);
  glfwMakeContextCurrent(window);
  glfwSetWindowUserPointer(window, &data);
  setVSync(true);

  // Set GLFW callbacks
  glfwSetWindowSizeCallback(
      window, [](GLFWwindow* window, int width, int height) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.width = width;
        data.height = height;

        WindowResizeEvent event(width, height);
        data.eventCallback(event);
      });

  glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    WindowCloseEvent event;
    data.eventCallback(event);
  });

  glfwSetKeyCallback(
      window, [](GLFWwindow* window, int key, [[maybe_unused]] int scancode,
                 int action, [[maybe_unused]] int mods) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        switch (action) {
          case GLFW_PRESS: {
            KeyPressedEvent event(key, 0);
            data.eventCallback(event);
            break;
          }
          case GLFW_RELEASE: {
            KeyReleasedEvent event(key);
            data.eventCallback(event);
            break;
          }
          case GLFW_REPEAT: {
            KeyPressedEvent event(key, 1);
            data.eventCallback(event);
            break;
          }
        }
      });

  glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button,
                                        int action, [[maybe_unused]] int mods) {
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

    switch (action) {
      case GLFW_PRESS: {
        MouseButtonPressedEvent event(button);
        data.eventCallback(event);
        break;
      }
      case GLFW_RELEASE: {
        MouseButtonReleasedEvent event(button);
        data.eventCallback(event);
        break;
      }
    }
  });

  glfwSetScrollCallback(
      window, [](GLFWwindow* window, double xOffset, double yOffset) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        MouseScrolledEvent event((float)xOffset, (float)yOffset);
        data.eventCallback(event);
      });

  glfwSetCursorPosCallback(
      window, [](GLFWwindow* window, double xPos, double yPos) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        MouseMovedEvent event((float)xPos, (float)yPos);
        data.eventCallback(event);
      });
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

  data.vSync = enabled;
}

bool Window::isVSync() const { return data.vSync; }

}  // namespace TE