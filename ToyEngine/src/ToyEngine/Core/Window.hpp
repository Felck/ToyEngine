#pragma once

#include <GLFW/glfw3.h>

#include "ToyEngine/Events/Event.hpp"
#include "tepch.hpp"

namespace TE {

struct WindowProps {
  std::string Title;
  uint32_t Width;
  uint32_t Height;

  WindowProps(const std::string& title = "ToyEngine", uint32_t width = 1280,
              uint32_t height = 720)
      : Title(title), Width(width), Height(height) {}
};

class Window {
 public:
  using EventCallbackFn = std::function<void(Event&)>;

  Window(const WindowProps& props);
  ~Window();

  void onUpdate();

  inline uint32_t getWidth() const { return data.width; }
  inline uint32_t getHeight() const { return data.height; }

  inline void setEventCallback(const EventCallbackFn& callback) {
    data.eventCallback = callback;
  }

  inline void* getNativeWindow() const { return window; }

  static Window* create(const WindowProps& props = WindowProps());

 private:
  void init(const WindowProps& props);
  void shutdown();

  GLFWwindow* window;

  struct WindowData {
    std::string title;
    uint32_t width, height;

    EventCallbackFn eventCallback;
  };

  WindowData data;
};

}  // namespace TE