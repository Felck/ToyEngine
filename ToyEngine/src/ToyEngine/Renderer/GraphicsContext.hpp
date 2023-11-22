#pragma once

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include "ToyEngine/Renderer/Shader.hpp"
#include "ToyEngine/Renderer/SwapChain.hpp"
#include "ToyEngine/Renderer/VertexArray.hpp"
#include "tepch.hpp"

namespace TE {
class GraphicsContext {
 public:
  GraphicsContext(GLFWwindow* native_window);
  ~GraphicsContext();

  inline vk::Device getDevice() const { return device; };
  inline vk::PhysicalDevice getGPU() const { return gpu; };
  inline vk::CommandPool getCommandPool() const { return command_pool; };
  inline vk::Queue getQueue() const { return queue; };

  void drawFrame();

 private:
  void initVulkan();
  void cleanupVulkan();

  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createDevice();
  void createRenderPass();
  void createGraphicsPipeline();
  void createFrameData();
  void recordCommandBuffer(vk::CommandBuffer buffer, uint32_t image_index);

  struct FrameData {
    vk::CommandPool command_pool;
    vk::CommandBuffer command_buffer;
    vk::Fence submit_fence;
    vk::Semaphore acquire_semaphore;
    vk::Semaphore release_semaphore;
  };

  GLFWwindow* window;
  uint32_t max_frames_in_flight;
  uint32_t current_frame = 0;

  vk::Instance instance;
  vk::PhysicalDevice gpu;
  vk::Device device;
  vk::Queue queue;
  vk::CommandPool command_pool;  // transient command pool
  vk::SurfaceKHR surface;
  vk::RenderPass render_pass;
  vk::PipelineLayout pipeline_layout;
  vk::Pipeline graphics_pipeline;
  SwapChain swapchain;
  uint32_t graphics_queue_index;
  vk::DebugUtilsMessengerEXT debug_messenger;

  std::vector<FrameData> frame_data;

  std::unique_ptr<VertexArray> vertex_array;

  friend class SwapChain;
};
}  // namespace TE