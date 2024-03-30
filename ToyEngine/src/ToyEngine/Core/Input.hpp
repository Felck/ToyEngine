#pragma once

#include "GLFW/glfw3.h"
#include "ToyEngine/Core/KeyCodes.hpp"

namespace TE {
class Input {
 public:
  static void init(GLFWwindow* window);
  static bool isKeyPressed(KeyCode keyCode);
  static bool isMouseButtonPressed(MouseCode button);
  static std::pair<float, float> getMousePosition();
  static float getMouseX();
  static float getMouseY();

 private:
  static GLFWwindow* window;
};
}  // namespace TE