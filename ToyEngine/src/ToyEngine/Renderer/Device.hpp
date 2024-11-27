#pragma once

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

namespace TE {
class Device {
 public:
  Device(GLFWwindow* window);
  ~Device();

  inline vk::Instance getInstance() const { return instance; }
  inline vk::SurfaceKHR getSurface() const { return surface; }
  inline vk::PhysicalDevice getGPU() const { return physical_device; }
  inline vk::Device getDevice() const { return logical_device; }
  inline vk::Queue getQueue() const { return queue; }
  inline uint32_t getGraphicsQueueIndex() const { return graphics_queue_index; }
  inline const vk::PhysicalDeviceProperties& getProperties() const { return properties; }

 private:
  void createInstance();
  void createSurface(GLFWwindow* window);
  void pickPhysicalDevice();
  void createLogicalDevice();

  vk::Instance instance;
  vk::SurfaceKHR surface;
  vk::PhysicalDevice physical_device;
  vk::Device logical_device;
  vk::Queue queue;
  vk::DebugUtilsMessengerEXT debug_messenger;
  uint32_t graphics_queue_index;
  vk::PhysicalDeviceProperties properties;
};
}  // namespace TE