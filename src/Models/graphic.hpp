#pragma once

#include <functional>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include "../Utils/VkResultThrowable.hpp"

#include "../Vulkan/framebuffer.hpp"
#include "../Vulkan/renderPass.hpp"

#include <stdio.h>
#include <vector>

class graphicPlus;

class graphic {
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

  uint32_t currentImageIndex = 0;

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

  std::vector<std::function<void()>> createSwapchainCallbacks;
  std::vector<std::function<void()>> destroySwapchainCallbacks;
  std::vector<std::function<void()>> createDeviceCallbacks;
  std::vector<std::function<void()>> destroyDeviceCallbacks;

  VkDebugUtilsMessengerEXT debugMessenger;

  //--------------------
  graphic() = default;
  graphic(graphic &&) = delete;
  graphic operator&(const graphic &) = delete;
  ~graphic();
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
  inline uint32_t CurrentImageIndex() const { return currentImageIndex; }
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

  // 该函数用于获取交换链图像索引到currentImageIndex，以及在需要重建交换链时调用RecreateSwapchain()、重建交换链后销毁旧交换链
  VkResultThrowable SwapImage(VkSemaphore semaphore_imageIsAvailable);

  // 该函数用于将命令缓冲区提交到用于图形的队列
  VkResultThrowable
  SubmitCommandBuffer_Graphics(VkSubmitInfo &submitInfo,
                               VkFence fence = VK_NULL_HANDLE) const;

  // 该函数用于在渲染循环中将命令缓冲区提交到图形队列的常见情形
  VkResultThrowable SubmitCommandBuffer_Graphics(
      VkCommandBuffer commandBuffer,
      VkSemaphore semaphore_imageIsAvailable = VK_NULL_HANDLE,
      VkSemaphore semaphore_renderFinished = VK_NULL_HANDLE,
      VkFence fence = VK_NULL_HANDLE,
      VkPipelineStageFlags waitDstStage_imageIsAvailable =
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) const;

  // 该函数用于将命令缓冲区提交到用于图形的队列，且只使用栅栏的常见情形
  VkResultThrowable
  SubmitCommandBuffer_Graphics(VkCommandBuffer commandBuffer,
                               VkFence fence = VK_NULL_HANDLE) const;

  // 该函数用于将命令缓冲区提交到用于计算的队列
  VkResultThrowable
  SubmitCommandBuffer_Compute(VkSubmitInfo &submitInfo,
                              VkFence fence = VK_NULL_HANDLE) const;

  // 该函数用于将命令缓冲区提交到用于计算的队列，且只使用栅栏的常见情形
  VkResultThrowable
  SubmitCommandBuffer_Compute(VkCommandBuffer commandBuffer,
                              VkFence fence = VK_NULL_HANDLE) const;

  // 该函数用于在渲染循环中呈现图像的常见情形
  VkResultThrowable
  PresentImage(VkSemaphore semaphore_renderFinished = VK_NULL_HANDLE);

  VkResultThrowable PresentImage(VkPresentInfoKHR &presentInfo);

  void CmdTransferImageOwnership(VkCommandBuffer commandBuffer) const;

  VkResultThrowable SubmitCommandBuffer_Presentation(
      VkCommandBuffer commandBuffer,
      VkSemaphore semaphore_renderingIsOver = VK_NULL_HANDLE,
      VkSemaphore semaphore_ownershipIsTransfered = VK_NULL_HANDLE,
      VkFence fence = VK_NULL_HANDLE) const;

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

  inline void AddCreateSwapchainCallback(std::function<void()> callback) {
    AddCallback(createSwapchainCallbacks, callback, "CreateSwapchainCallback");
  }

  inline void AddDestroySwapchainCallback(std::function<void()> callback) {
    AddCallback(destroySwapchainCallbacks, callback,
                "DestroySwapchainCallback");
  }

  inline void AddCreateDeviceCallback(std::function<void()> callback) {
    AddCallback(createDeviceCallbacks, callback, "CreateDeviceCallback");
  }

  inline void AddDestroyDeviceCallback(std::function<void()> callback) {
    AddCallback(destroyDeviceCallbacks, callback, "DestroyDeviceCallback");
  }

  inline void ClearCreateSwapchainCallbacks() {
#ifndef NDEBUG
    printf("[ graphicsBase ] DEBUG: Clearing createSwapchainCallbacks\n");
#endif
    createSwapchainCallbacks.clear();
  }

  inline void ClearDestroySwapchainCallbacks() {
#ifndef NDEBUG
    printf("[ graphicsBase ] DEBUG: Clearing destroySwapchainCallbacks\n");
#endif
    destroySwapchainCallbacks.clear();
  }

  inline void ClearCreateDeviceCallbacks() {
#ifndef NDEBUG
    printf("[ graphicsBase ] DEBUG: Clearing createDeviceCallbacks\n");
#endif
    createDeviceCallbacks.clear();
  }

  inline void ClearDestroyDeviceCallbacks() {
#ifndef NDEBUG
    printf("[ graphicsBase ] DEBUG: Clearing destroyDeviceCallbacks\n");
#endif
    destroyDeviceCallbacks.clear();
  }

  // 该函数用于等待逻辑设备空闲
  VkResultThrowable WaitIdle() const;
  // 该函数用于重建逻辑设备
  VkResultThrowable RecreateDevice(VkDeviceCreateFlags flags = 0);

  void Terminate();
  // 单例
  static graphic &Singleton();
  static graphicPlus &Plus();
};
