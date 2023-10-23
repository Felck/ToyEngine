#include "Shader.hpp"

#include <vulkan/vulkan.hpp>

#include "tepch.hpp"

namespace TE {

Shader::Shader(const std::string& filename, ShaderType type) : type(type) {
  std::ifstream file("shaders/" + filename + ".spv", std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file: " + filename);
  }

  size_t file_size = static_cast<size_t>(file.tellg());
  code.resize(file_size);

  file.seekg(0);
  file.read(code.data(), file_size);

  file.close();
}

vk::PipelineShaderStageCreateInfo Shader::getStageCreateInfo(vk::Device& device) {
  vk::ShaderStageFlagBits stage;
  switch (type) {
    case ShaderType::VERTEX:
      stage = vk::ShaderStageFlagBits::eVertex;
      break;
    case ShaderType::FRAGMENT:
      stage = vk::ShaderStageFlagBits::eFragment;
      break;
    default:
      throw std::runtime_error("ShaderType not implemented!");
  }

  vk::ShaderModuleCreateInfo module_info{
      .codeSize = code.size(),
      .pCode = reinterpret_cast<uint32_t*>(code.data()),  // TODO: remove UB
  };

  vk::ShaderModule module = device.createShaderModule(module_info);

  vk::PipelineShaderStageCreateInfo stage_info{
      .stage = stage,
      .module = module,
      .pName = "main",
  };

  return stage_info;
}
}  // namespace TE
