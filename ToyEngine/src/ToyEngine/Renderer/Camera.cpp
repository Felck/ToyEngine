// ToyEngine/src/ToyEngine/Renderer/Camera.cpp
#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace TE {

Camera::Camera(glm::vec3 position, glm::vec3 lookAt) {
  this->position = position;
  projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f);
  view = glm::lookAt(position, lookAt, glm::vec3(0.0f, 1.0f, 0.0f));
}

}  // namespace TE
