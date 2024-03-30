#include <glm/glm.hpp>

#include "glm/ext/matrix_transform.hpp"

namespace TE {
class Camera {
 public:
  Camera(glm::vec3 position, glm::vec3 lookAt);
  inline const glm::mat4& getViewProjection() {
    viewProjection = projection * view;
    return viewProjection;
  }
  inline void move(float x, float y, float z) { view = glm::translate(view, glm::vec3(x, -y, z)); };

 private:
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 viewProjection;
  glm::vec3 position;
};

}  // namespace TE