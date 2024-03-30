#include "ToyEngine/Core/Input.hpp"

#include "GLFW/glfw3.h"
#include "tepch.hpp"

namespace TE {

GLFWwindow* Input::window = nullptr;

void Input::init(GLFWwindow* window) {
  assert(Input::window == nullptr);
  Input::window = window;
}

bool Input::isKeyPressed(int keyCode) {
  auto state = glfwGetKey(Input::window, keyCode);
  return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::isMouseButtonPressed(int button) {
  auto state = glfwGetMouseButton(Input::window, button);
  return state == GLFW_PRESS;
}

std::pair<float, float> Input::getMousePosition() {
  double xpos, ypos;
  glfwGetCursorPos(Input::window, &xpos, &ypos);
  return {static_cast<float>(xpos), static_cast<float>(ypos)};
}

float Input::getMouseX() {
  auto [x, _] = getMousePosition();
  return x;
}

float Input::getMouseY() {
  auto [_, y] = getMousePosition();
  return y;
}
}  // namespace TE
