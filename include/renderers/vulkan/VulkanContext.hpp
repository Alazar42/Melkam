#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <vector>
#include <optional>
#include <algorithm>

// Vulkan Hardware Context Manager (Instance, Physical Device, Logical Device, VMA, Swapchain)
class VulkanContext {
public:
  VkInstance instance = VK_NULL_HANDLE;
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  VkQueue graphicsQueue = VK_NULL_HANDLE;
  VkQueue presentQueue = VK_NULL_HANDLE;
  uint32_t graphicsQueueFamily = 0;
  uint32_t presentQueueFamily = 0;
  VmaAllocator allocator = VK_NULL_HANDLE;

  bool initialized = false;

  static VulkanContext &get() {
    static VulkanContext s_instance;
    return s_instance;
  }

  bool init(SDL_Window *window) {
    if (initialized) return true;

    // 1. Initialize Volk meta-loader
    if (volkInitialize() != VK_SUCCESS) {
      std::cerr << "[MelkamEngine::Vulkan] Failed to initialize Volk meta-loader." << std::endl;
      return false;
    }

    // 2. Query required SDL extensions
    uint32_t extCount = 0;
    const char *const *sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extCount);
    if (!sdlExtensions) {
      std::cerr << "[MelkamEngine::Vulkan] Failed to retrieve SDL Vulkan extensions: " << SDL_GetError() << std::endl;
      return false;
    }

    std::vector<const char *> extensions(sdlExtensions, sdlExtensions + extCount);

    // 3. Create Vulkan Instance
    VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pApplicationName = "MelkamEngine 3D";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "MelkamEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
      std::cerr << "[MelkamEngine::Vulkan] Failed to create Vulkan instance." << std::endl;
      return false;
    }

    volkLoadInstance(instance);

    // 4. Create Window Surface via SDL3
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
      std::cerr << "[MelkamEngine::Vulkan] Failed to create SDL Vulkan surface: " << SDL_GetError() << std::endl;
      return false;
    }

    // 5. Select Physical Device (GPU)
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
      std::cerr << "[MelkamEngine::Vulkan] No Vulkan-compatible GPUs found." << std::endl;
      return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // Prefer discrete GPU
    for (const auto &dev : devices) {
      VkPhysicalDeviceProperties props;
      vkGetPhysicalDeviceProperties(dev, &props);
      if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        physicalDevice = dev;
        std::cout << "[MelkamEngine::Vulkan] Selected Discrete GPU: " << props.deviceName << std::endl;
        break;
      }
    }
    if (physicalDevice == VK_NULL_HANDLE) {
      physicalDevice = devices[0];
      VkPhysicalDeviceProperties props;
      vkGetPhysicalDeviceProperties(physicalDevice, &props);
      std::cout << "[MelkamEngine::Vulkan] Selected GPU: " << props.deviceName << std::endl;
    }

    // 6. Find Queue Families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    bool foundGraphics = false;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        graphicsQueueFamily = i;
        foundGraphics = true;
      }
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
      if (presentSupport) {
        presentQueueFamily = i;
      }
    }

    if (!foundGraphics) {
      std::cerr << "[MelkamEngine::Vulkan] Could not find graphics queue family." << std::endl;
      return false;
    }

    // 7. Create Logical Device
    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::vector<uint32_t> uniqueFamilies = {graphicsQueueFamily};
    if (presentQueueFamily != graphicsQueueFamily) {
      uniqueFamilies.push_back(presentQueueFamily);
    }

    for (uint32_t family : uniqueFamilies) {
      VkDeviceQueueCreateInfo queueInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
      queueInfo.queueFamilyIndex = family;
      queueInfo.queueCount = 1;
      queueInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueInfo);
    }

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceCreateInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
      std::cerr << "[MelkamEngine::Vulkan] Failed to create logical Vulkan device." << std::endl;
      return false;
    }

    volkLoadDevice(device);
    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentQueueFamily, 0, &presentQueue);

    // 8. Initialize AMD Vulkan Memory Allocator (VMA)
    VmaVulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;

    if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS) {
      std::cerr << "[MelkamEngine::Vulkan] Failed to initialize Vulkan Memory Allocator (VMA)." << std::endl;
      return false;
    }

    initialized = true;
    std::cout << "[MelkamEngine::Vulkan] Hardware Rendering Context Initialized Successfully (Godot 4 Architecture)!" << std::endl;
    return true;
  }

  void shutdown() {
    if (!initialized) return;

    if (allocator != VK_NULL_HANDLE) {
      vmaDestroyAllocator(allocator);
      allocator = VK_NULL_HANDLE;
    }
    if (device != VK_NULL_HANDLE) {
      vkDestroyDevice(device, nullptr);
      device = VK_NULL_HANDLE;
    }
    if (surface != VK_NULL_HANDLE) {
      vkDestroySurfaceKHR(instance, surface, nullptr);
      surface = VK_NULL_HANDLE;
    }
    if (instance != VK_NULL_HANDLE) {
      vkDestroyInstance(instance, nullptr);
      instance = VK_NULL_HANDLE;
    }

    initialized = false;
  }
};
