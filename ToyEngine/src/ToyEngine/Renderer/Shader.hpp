#pragma once

#include <vulkan/vulkan.hpp>

#include "tepch.hpp"

namespace TE {

enum class ShaderType {
  VERTEX,
  FRAGMENT,
  COMPUTE,
};

class Shader {
 public:
  Shader(const std::string& filename, ShaderType type);

  vk::PipelineShaderStageCreateInfo getStageCreateInfo(vk::Device& device);

 private:
  std::vector<char> code;
  ShaderType type;
};

}  // namespace TE