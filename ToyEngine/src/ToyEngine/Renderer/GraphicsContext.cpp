#include "GraphicsContext.hpp"

#include "ToyEngine/Core/Application.hpp"

// instantiate the default dispatcher
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace {
#ifdef ENABLE_VALIDATION_LAYERS
const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

bool validateLayers() {
  auto available = vk::enumerateInstanceLayerProperties();

  return !std::any_of(VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end(), [&available](auto layer) {
    return !std::any_of(available.begin(), available.end(),
                        [&layer](auto const& lp) { return strcmp(lp.layerName, layer) == 0; });
  });

  auto unavailable =
      std::find_if(VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end(), [&available](auto layer) {
        return std::find_if(available.begin(), available.end(), [&layer](auto const& l) {
                 return strcmp(l.layerName, layer) == 0;
               }) == available.end();
      });

  return (unavailable == VALIDATION_LAYERS.end());
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
  return !std::any_of(required.begin(), required.end(), [&available](auto extension) {
    return !std::any_of(available.begin(), available.end(), [&extension](auto const& ep) {
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

GraphicsContext::GraphicsContext(Window& window)
    : window(static_cast<GLFWwindow*>(window.getNativeWindow())) {
  initVulkan();
}

GraphicsContext::~GraphicsContext() { cleanupVulkan(); }

void GraphicsContext::initVulkan() {
  // initialize function pointers
  VULKAN_HPP_DEFAULT_DISPATCHER.init();
  createInstance();
  createSurface();
  pickPhysicalDevice();
  createDevice();
  createSwapChain();
}

void TE::GraphicsContext::cleanupVulkan() {
  if (swapchain_data.swapchain) {
    device.destroySwapchainKHR(swapchain_data.swapchain);
  }

  if (device) {
    device.destroy();
  }

  if (surface) {
    instance.destroySurfaceKHR(surface);
  }

  if (debug_messenger) {
    instance.destroyDebugUtilsMessengerEXT(debug_messenger);
  }

  instance.destroy();
}

void GraphicsContext::createInstance() {
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
      .apiVersion = VK_API_VERSION_1_0,
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

void GraphicsContext::createSurface() {
  VkSurfaceKHR s;
  if (glfwCreateWindowSurface(instance, window, nullptr, &s) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create window surface.");
  }
  surface = vk::SurfaceKHR(s);
}

void GraphicsContext::pickPhysicalDevice() {
  std::vector<vk::PhysicalDevice> gpus = instance.enumeratePhysicalDevices();

  bool found_graphics_queue_index = false;
  for (size_t i = 0; i < gpus.size() && !found_graphics_queue_index; i++) {
    gpu = gpus[i];

    auto queue_family_properties = gpu.getQueueFamilyProperties();

    if (queue_family_properties.empty()) {
      throw std::runtime_error("No queue family found.");
    }

    for (uint32_t j = 0; j < static_cast<uint32_t>(queue_family_properties.size()); j++) {
      auto supports_present = gpu.getSurfaceSupportKHR(j, surface);

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

void GraphicsContext::createDevice() {
  std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  auto device_extensions = gpu.enumerateDeviceExtensionProperties();
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
  device = gpu.createDevice(device_info);

  // initialize function pointers for device
  VULKAN_HPP_DEFAULT_DISPATCHER.init(device);

  queue = device.getQueue(graphics_queue_index, 0);
}

void GraphicsContext::createSwapChain() {
  auto capabilities = gpu.getSurfaceCapabilitiesKHR(surface);

  // format
  auto format = selectSurfaceFormat(
      {vk::Format::eR8G8B8A8Srgb, vk::Format::eB8G8R8A8Srgb, vk::Format::eA8B8G8R8SrgbPack32});

  // extent
  vk::Extent2D extent;
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    extent = capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    extent.width = std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
    extent.height = std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height);
  }

  // images
  uint32_t image_count = capabilities.minImageCount + 1;
  if ((capabilities.maxImageCount > 0) && (image_count > capabilities.maxImageCount)) {
    image_count = capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR swapchain_create_info{
      .surface = surface,
      .minImageCount = image_count,
      .imageFormat = format.format,
      .imageColorSpace = format.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = capabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = vk::PresentModeKHR::eFifo,
      .clipped = true,
  };
  swapchain_data.swapchain = device.createSwapchainKHR(swapchain_create_info);
  swapchain_data.swap_chain_images = device.getSwapchainImagesKHR(swapchain_data.swapchain);
  swapchain_data.format = format.format;
  swapchain_data.extent = extent;
}

vk::SurfaceFormatKHR GraphicsContext::selectSurfaceFormat(
    std::vector<vk::Format> const& preferred) {
  auto available = gpu.getSurfaceFormatsKHR(surface);

  auto it =
      std::find_if(available.begin(), available.end(), [&preferred](auto const& surface_format) {
        return std::any_of(preferred.begin(), preferred.end(), [&surface_format](auto format) {
          return format == surface_format.format;
        });
      });

  return it != available.end() ? *it : available[0];
}
}  // namespace TE
