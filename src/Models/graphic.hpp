#pragma once

#define GLFW_INCLUDE_VULKAN

#include "../Utils/VkResultThrowable.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

class graphicsBase {
  uint32_t apiVersion = VK_API_VERSION_1_0;
  VkInstance instance;
  VkPhysicalDevice physicalDevice;
  VkPhysicalDeviceProperties physicalDeviceProperties;
  VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
  std::vector<VkPhysicalDevice> availablePhysicalDevices;

  VkDevice device;
  uint32_t queueFamilyIndex_graphics = VK_QUEUE_FAMILY_IGNORED;
  uint32_t queueFamilyIndex_presentation = VK_QUEUE_FAMILY_IGNORED;
  uint32_t queueFamilyIndex_compute = VK_QUEUE_FAMILY_IGNORED;
  VkQueue queue_graphics;
  VkQueue queue_presentation;
  VkQueue queue_compute;

  VkSurfaceKHR surface;
  std::vector<VkSurfaceFormatKHR> availableSurfaceFormats;

  VkSwapchainKHR swapchain;
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;
  VkSwapchainCreateInfoKHR swapchainCreateInfo = {};

  std::vector<const char *> instanceLayers;
  std::vector<const char *> instanceExtensions;
  std::vector<const char *> deviceExtensions;

  std::vector<void (*)()> createSwapchainCallbacks;
  std::vector<void (*)()> destroySwapchainCallbacks;
  std::vector<void (*)()> createDeviceCallbacks;
  std::vector<void (*)()> destroyDeviceCallbacks;

  VkDebugUtilsMessengerEXT debugMessenger;

  //--------------------
  graphicsBase() = default;
  graphicsBase(graphicsBase &&) = delete;
  graphicsBase operator&(const graphicsBase &) = delete;
  ~graphicsBase();
  // Non const函数
  VkResultThrowable GetQueueFamilyIndices(VkPhysicalDevice physicalDevice,
                                          bool enableGraphicsQueue,
                                          bool enableComputeQueue,
                                          uint32_t (&queueFamilyIndices)[3]);

  VkResultThrowable CreateDebugMessenger();
  VkResultThrowable CreateSwapchain_Internal();

public:
  const constexpr static VkOffset2D defaultPosition = {0, 0};
  const constexpr static VkExtent2D defaultSize = {1920, 1080};

  // Getter
  inline uint32_t ApiVersion() const { return apiVersion; }
  inline VkInstance Instance() const { return instance; }
  inline VkPhysicalDevice PhysicalDevice() const { return physicalDevice; }
  inline const VkPhysicalDeviceProperties &PhysicalDeviceProperties() const {
    return physicalDeviceProperties;
  }
  inline const VkPhysicalDeviceMemoryProperties &
  PhysicalDeviceMemoryProperties() const {
    return physicalDeviceMemoryProperties;
  }
  inline VkPhysicalDevice AvailablePhysicalDevice(uint32_t index) const {
    return availablePhysicalDevices[index];
  }
  inline uint32_t AvailablePhysicalDeviceCount() const {
    return uint32_t(availablePhysicalDevices.size());
  }

  inline VkDevice Device() const { return device; }
  inline uint32_t QueueFamilyIndex_Graphics() const {
    return queueFamilyIndex_graphics;
  }
  inline uint32_t QueueFamilyIndex_Presentation() const {
    return queueFamilyIndex_presentation;
  }
  inline uint32_t QueueFamilyIndex_Compute() const {
    return queueFamilyIndex_compute;
  }
  inline VkQueue Queue_Graphics() const { return queue_graphics; }
  inline VkQueue Queue_Presentation() const { return queue_presentation; }
  inline VkQueue Queue_Compute() const { return queue_compute; }

  inline VkSurfaceKHR Surface() const { return surface; }
  inline const VkFormat &AvailableSurfaceFormat(uint32_t index) const {
    return availableSurfaceFormats[index].format;
  }
  inline const VkColorSpaceKHR &
  AvailableSurfaceColorSpace(uint32_t index) const {
    return availableSurfaceFormats[index].colorSpace;
  }
  inline uint32_t AvailableSurfaceFormatCount() const {
    return uint32_t(availableSurfaceFormats.size());
  }

  inline VkSwapchainKHR Swapchain() const { return swapchain; }
  inline VkImage SwapchainImage(uint32_t index) const {
    return swapchainImages[index];
  }
  inline VkImageView SwapchainImageView(uint32_t index) const {
    return swapchainImageViews[index];
  }
  inline uint32_t SwapchainImageCount() const {
    return uint32_t(swapchainImages.size());
  }
  inline const VkSwapchainCreateInfoKHR &SwapchainCreateInfo() const {
    return swapchainCreateInfo;
  }

  inline const std::vector<const char *> &InstanceLayers() const {
    return instanceLayers;
  }
  inline const std::vector<const char *> &InstanceExtensions() const {
    return instanceExtensions;
  }
  inline const std::vector<const char *> &DeviceExtensions() const {
    return deviceExtensions;
  }

  // Const函数
  VkResultThrowable CheckInstanceLayers(const char **layersToCheck,
                                        int length) const;
  VkResultThrowable
  CheckInstanceExtensions(const char **extensionsToCheck, int length,
                          const char *layerName = nullptr) const;
  VkResultThrowable
  CheckDeviceExtensions(const char **extensionsToCheck, int length,
                        const char *layerName = nullptr) const;

  // Non-const函数
  inline void AddInstanceLayer(const char *layerName) {
    instanceLayers.push_back(layerName);
  }
  inline void AddInstanceExtension(const char *extensionName) {
    instanceExtensions.push_back(extensionName);
  }
  inline void AddDeviceExtension(const char *extensionName) {
    deviceExtensions.push_back(extensionName);
  }

  inline VkResultThrowable UseLatestApiVersion() {
    if (vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion")) {
      return vkEnumerateInstanceVersion(&apiVersion);
    }
    return VK_SUCCESS;
  }

  VkResultThrowable CreateInstance(VkInstanceCreateFlags flags = 0);

  inline void Surface(VkSurfaceKHR surface) {
    if (!this->surface)
      this->surface = surface;
  }

  VkResultThrowable GetPhysicalDevices();
  VkResultThrowable DeterminePhysicalDevice(uint32_t deviceIndex,
                                            bool enableGraphicsQueue,
                                            bool enableComputeQueue);
  VkResultThrowable CreateDevice(VkDeviceCreateFlags flags = 0);
  VkResultThrowable GetSurfaceFormats();

  VkResultThrowable SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat);

  VkResultThrowable CreateSwapchain(bool limitFrameRate = true,
                                    VkSwapchainCreateFlagsKHR flags = 0);

  VkResultThrowable RecreateSwapchain();

  inline void InstanceLayers(const std::vector<const char *> &layerNames) {
    instanceLayers = layerNames;
  }
  inline void
  InstanceExtensions(const std::vector<const char *> &extensionNames) {
    instanceExtensions = extensionNames;
  }
  inline void
  DeviceExtensions(const std::vector<const char *> &extensionNames) {
    deviceExtensions = extensionNames;
  }

  inline void AddCreateSwapchainCallback(void (*callback)()) {
    createSwapchainCallbacks.push_back(callback);
  }

  inline void AddDestroySwapchainCallback(void (*callback)()) {
    destroySwapchainCallbacks.push_back(callback);
  }

  inline void AddCreateDeviceCallback(void (*callback)()) {
    createDeviceCallbacks.push_back(callback);
  }

  inline void AddDestroyDeviceCallback(void (*callback)()) {
    destroyDeviceCallbacks.push_back(callback);
  }

  // 该函数用于等待逻辑设备空闲
  VkResultThrowable WaitIdle() const;
  // 该函数用于重建逻辑设备
  VkResultThrowable RecreateDevice(VkDeviceCreateFlags flags = 0);

  void Terminate();
  // 单例
  static graphicsBase &Singleton();
};
