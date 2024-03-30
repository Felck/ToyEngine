#include "GLFW/glfw3.h"
namespace TE {
class Input {
 public:
  static void init(GLFWwindow* window);
  static bool isKeyPressed(int keyCode);
  static bool isMouseButtonPressed(int button);
  static std::pair<float, float> getMousePosition();
  static float getMouseX();
  static float getMouseY();

 private:
  static GLFWwindow* window;
};
}  // namespace TE