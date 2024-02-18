#include "Device.hpp"

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

namespace {
#ifdef ENABLE_VALIDATION_LAYERS
const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

bool validateLayers() {
  auto available = vk::enumerateInstanceLayerProperties();
  return std::all_of(VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end(),
                     [&available](const char* layer) {
                       return std::find_if(available.begin(), available.end(),
                                           [layer](const vk::LayerProperties& props) {
                                             return strcmp(props.layerName, layer) == 0;
                                           }) != available.end();
                     });
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData) {
  std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}
#endif

bool validateExtensions(const std::vector<const char*>& required,
                        const std::vector<vk::ExtensionProperties>& available) {
  return !std::any_of(required.begin(), required.end(), [&available](const auto extension) {
    return !std::any_of(available.begin(), available.end(), [&extension](const auto& ep) {
      return strcmp(ep.extensionName, extension) == 0;
    });
  });
}

std::vector<const char*> getRequiredInstanceExtensions() {
  // glfw extensions
  uint32_t glfw_extension_count = 0;
  const char** glfw_extensions;
  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

#ifdef ENABLE_VALIDATION_LAYERS
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

  return extensions;
}

}  // namespace
namespace TE {
Device::Device(GLFWwindow* window) {
  // initialize function pointers
  VULKAN_HPP_DEFAULT_DISPATCHER.init();

  createInstance();
  createSurface(window);
  pickPhysicalDevice();
  createLogicalDevice();
}

Device::~Device() {
  if (logical_device) {
    logical_device.destroy();
  }

  if (surface) {
    instance.destroySurfaceKHR(surface);
  }

  if (debug_messenger) {
    instance.destroyDebugUtilsMessengerEXT(debug_messenger);
  }

  instance.destroy();
}

void Device::createInstance() {
#ifdef ENABLE_VALIDATION_LAYERS
  if (!validateLayers()) {
    throw std::runtime_error("Validation layers requested, but not available.");
  }
#endif

  // TODO: query application/engine data
  vk::ApplicationInfo application_info{
      .pApplicationName = "VKApp",
      .applicationVersion = 1,
      .pEngineName = "ToyEngine",
      .engineVersion = 1,
      .apiVersion = VK_API_VERSION_1_3,
  };

  auto extensions = getRequiredInstanceExtensions();
  auto available_extensions = vk::enumerateInstanceExtensionProperties();
  if (!validateExtensions(extensions, available_extensions)) {
    throw std::runtime_error("Required instance extensions are missing.");
  }

  vk::InstanceCreateInfo instance_create_info{
      .pApplicationInfo = &application_info,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
  };

#ifdef ENABLE_VALIDATION_LAYERS
  instance_create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
  instance_create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

  vk::DebugUtilsMessengerCreateInfoEXT debug_utils_create_info{
      .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                         vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
      .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                     vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
      .pfnUserCallback = debugCallback,
  };

  instance_create_info.pNext = &debug_utils_create_info;
#else
  instance_create_info.enabledLayerCount = 0;
#endif

  instance = vk::createInstance(instance_create_info);

  // initialize function pointers for instance
  VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

#ifdef ENABLE_VALIDATION_LAYERS
  debug_messenger = instance.createDebugUtilsMessengerEXT(debug_utils_create_info);
#endif
}

void Device::createSurface(GLFWwindow* window) {
  VkSurfaceKHR s;
  if (glfwCreateWindowSurface(instance, window, nullptr, &s) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface.");
  }
  surface = vk::SurfaceKHR(s);
}

void Device::pickPhysicalDevice() {
  std::vector<vk::PhysicalDevice> gpus = instance.enumeratePhysicalDevices();

  bool found_graphics_queue_index = false;
  for (size_t i = 0; i < gpus.size() && !found_graphics_queue_index; i++) {
    physical_device = gpus[i];

    auto queue_family_properties = physical_device.getQueueFamilyProperties();

    if (queue_family_properties.empty()) {
      throw std::runtime_error("No queue family found.");
    }

    for (uint32_t j = 0; j < static_cast<uint32_t>(queue_family_properties.size()); j++) {
      auto supports_present = physical_device.getSurfaceSupportKHR(j, surface);

      if ((queue_family_properties[j].queueFlags & vk::QueueFlagBits::eGraphics) &&
          supports_present) {
        graphics_queue_index = j;
        found_graphics_queue_index = true;
        break;
      }
    }
  }

  if (!found_graphics_queue_index) {
    throw std::runtime_error("Did not find suitable GPU.");
  }
}

void Device::createLogicalDevice() {
  std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  auto device_extensions = physical_device.enumerateDeviceExtensionProperties();
  if (!validateExtensions(extensions, device_extensions)) {
    throw std::runtime_error("Required device extensions are missing.");
  }

  float queue_priority = 1.0f;
  vk::DeviceQueueCreateInfo queue_info{
      .queueFamilyIndex = graphics_queue_index,
      .queueCount = 1,
      .pQueuePriorities = &queue_priority,
  };
  vk::DeviceCreateInfo device_info{
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queue_info,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
  };
  logical_device = physical_device.createDevice(device_info);

  // initialize function pointers for device
  VULKAN_HPP_DEFAULT_DISPATCHER.init(logical_device);

  queue = logical_device.getQueue(graphics_queue_index, 0);
}

}  // namespace TE