#pragma once

#include <GLFW/glfw3.h>

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "ToyEngine/Renderer/Allocator.hpp"
#include "ToyEngine/Renderer/Device.hpp"
#include "ToyEngine/Renderer/SwapChain.hpp"

namespace TE {
class GraphicsContext {
 public:
  GraphicsContext(GLFWwindow* window);
  ~GraphicsContext();

  inline static GraphicsContext& get() { return *instance; }

  inline VmaAllocator getAllocator() const { return allocator.getAllocator(); }
  inline vk::Instance getInstance() const { return device.getInstance(); }
  inline vk::Device getDevice() const { return device.getDevice(); }
  inline auto& getDeviceProperties() const { return device.getProperties(); }
  inline vk::PhysicalDevice getGPU() const { return device.getGPU(); }
  inline vk::CommandPool getCommandPool() const { return transient_command_pool; }
  inline vk::Queue getQueue() const { return device.getQueue(); }
  inline uint32_t getGraphicsQueueIndex() const { return device.getGraphicsQueueIndex(); }
  inline vk::DescriptorSet getDescriptorSet() const { return descriptor_set; }
  inline vk::PipelineLayout getPipelineLayout() const { return pipeline_layout; }
  inline const SwapChain& getSwapChain() const { return swapchain; }
  inline vk::CommandBuffer getCommandBuffer() const {
    return frame_data[current_frame].command_buffer;
  }

  void beginFrame();
  void endFrame();
  void beginPass();
  void endPass();

  inline void record(const std::invocable<vk::CommandBuffer> auto&& commands) const {
    commands(frame_data[current_frame].command_buffer);
  }

  inline void executeTransient(const std::invocable<vk::CommandBuffer> auto&& commands) const {
    vk::CommandBuffer command_buffer = beginTransientExecution();
    commands(command_buffer);
    endTransientExecution(command_buffer);
  }

 private:
  void createRenderPass();
  void createDescriptorSets();
  void createGraphicsPipeline();
  void createFrameData();

  vk::CommandBuffer beginTransientExecution() const;
  void endTransientExecution(vk::CommandBuffer cmd) const;

  struct FrameData {
    vk::CommandPool command_pool;
    vk::CommandBuffer command_buffer;
    vk::Fence submit_fence;
    vk::Semaphore acquire_semaphore;
  };

  Device device;
  Allocator allocator;
  SwapChain swapchain;
  vk::CommandPool transient_command_pool;
  vk::RenderPass render_pass;
  vk::DescriptorSetLayout descriptor_set_layout;
  vk::DescriptorPool descriptor_pool;
  vk::DescriptorSet descriptor_set;
  vk::PipelineLayout pipeline_layout;
  vk::Pipeline graphics_pipeline;

  std::vector<FrameData> frame_data;
  uint32_t max_frames_in_flight;
  uint32_t current_frame = 0;

  static GraphicsContext* instance;
  static constexpr uint32_t UNIFORM_BUFFER_COUNT = 1000;
  static constexpr uint32_t STORAGE_BUFFER_COUNT = 1000;
  static constexpr uint32_t TEXTURE_COUNT = 1000;
};
}  // namespace TE