#pragma once

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include "ToyEngine/Renderer/Device.hpp"
#include "ToyEngine/Renderer/Shader.hpp"
#include "ToyEngine/Renderer/SwapChain.hpp"
#include "ToyEngine/Renderer/VertexArray.hpp"
#include "tepch.hpp"

namespace TE {
class GraphicsContext {
 public:
  GraphicsContext(GLFWwindow* window);
  ~GraphicsContext();

  inline vk::Instance getInstance() const { return device.getInstance(); }
  inline vk::Device getDevice() const { return device.getDevice(); }
  inline vk::PhysicalDevice getGPU() const { return device.getGPU(); }
  inline vk::CommandPool getCommandPool() const { return transient_command_pool; }
  inline vk::Queue getQueue() const { return device.getQueue(); }
  inline uint32_t getGraphicsQueueIndex() const { return device.getGraphicsQueueIndex(); }
  inline const SwapChain& getSwapChain() const { return swapchain; }

  void drawFrame();

  template <typename F>
  void executeTransient(F const& commands) const {
    vk::CommandBuffer command_buffer = beginTransientExecution();
    commands(command_buffer);
    endTransientExecution(command_buffer);
  }

 private:
  void createRenderPass();
  void createGraphicsPipeline();
  void createFrameData();

  vk::CommandBuffer beginTransientExecution() const;
  void endTransientExecution(vk::CommandBuffer cmd) const;

  void recordCommandBuffer(vk::CommandBuffer buffer, uint32_t image_index);

  struct FrameData {
    vk::CommandPool command_pool;
    vk::CommandBuffer command_buffer;
    vk::Fence submit_fence;
    vk::Semaphore acquire_semaphore;
    vk::Semaphore release_semaphore;
  };

  Device device;
  SwapChain swapchain;
  vk::CommandPool transient_command_pool;
  vk::RenderPass render_pass;
  vk::PipelineLayout pipeline_layout;
  vk::Pipeline graphics_pipeline;

  std::vector<FrameData> frame_data;
  uint32_t max_frames_in_flight;
  uint32_t current_frame = 0;

  std::unique_ptr<VertexArray> vertex_array;
};
}  // namespace TE