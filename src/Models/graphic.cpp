#pragma once

#include <cstdint>
#define GLFW_INCLUDE_VULKAN

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <GLFW/glfw3.h>

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

  VkDebugUtilsMessengerEXT debugMessenger;

  //--------------------
  graphicsBase() = default;
  graphicsBase(graphicsBase &&) = delete;
  graphicsBase operator&(const graphicsBase &) = delete;
  ~graphicsBase() { /*待Ch1-4填充*/ }
  // Non const函数
  VkResult CreateSwapchain_Internal() { /*待Ch1-4填充*/ }
  VkResult GetQueueFamilyIndices(VkPhysicalDevice physicalDevice,
                                 bool enableGraphicsQueue,
                                 bool enableComputeQueue,
                                 uint32_t (&queueFamilyIndices)[3]) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             nullptr);
    if (!queueFamilyCount)
      return VK_RESULT_MAX_ENUM;
    std::vector<VkQueueFamilyProperties> queueFamilyPropertieses(
        queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             queueFamilyPropertieses.data());
    auto &[ig, ip, ic] = queueFamilyIndices;
    ig = ip = ic = VK_QUEUE_FAMILY_IGNORED;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
      // 这三个VkBool32变量指示是否可获取（指应该被获取且能获取）相应队列族索引
      VkBool32
          // 只在enableGraphicsQueue为true时获取支持图形操作的队列族的索引
          supportGraphics =
              enableGraphicsQueue &&
              queueFamilyPropertieses[i].queueFlags & VK_QUEUE_GRAPHICS_BIT,
          supportPresentation = false,
          // 只在enableComputeQueue为true时获取支持计算的队列族的索引
          supportCompute =
              enableComputeQueue &&
              queueFamilyPropertieses[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
      // 只在创建了window surface时获取支持呈现的队列族的索引
      if (surface)
        if (VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
                physicalDevice, i, surface, &supportPresentation)) {
          printf(
              "[ graphicsBase ] ERROR: Failed to determine if the queue family "
              "supports presentation!\nError code: %d\n",
              int32_t(result));
          return result;
        }
      // 若某队列族同时支持图形操作和计算
      if (supportGraphics && supportCompute) {
        // 若需要呈现，最好是三个队列族索引全部相同
        if (supportPresentation) {
          ig = ip = ic = i;
          break;
        }
        // 除非ig和ic都已取得且相同，否则将它们的值覆写为i，以确保两个队列族索引相同
        if (ig != ic || ig == VK_QUEUE_FAMILY_IGNORED)
          ig = ic = i;
        // 如果不需要呈现，那么已经可以break了
        if (!surface)
          break;
      }
      // 若任何一个队列族索引可以被取得但尚未被取得，将其值覆写为i
      if (supportGraphics && ig == VK_QUEUE_FAMILY_IGNORED)
        ig = i;
      if (supportPresentation && ip == VK_QUEUE_FAMILY_IGNORED)
        ip = i;
      if (supportCompute && ic == VK_QUEUE_FAMILY_IGNORED)
        ic = i;
    }
    if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphicsQueue ||
        ip == VK_QUEUE_FAMILY_IGNORED && surface ||
        ic == VK_QUEUE_FAMILY_IGNORED && enableComputeQueue)
      return VK_RESULT_MAX_ENUM;
    queueFamilyIndex_graphics = ig;
    queueFamilyIndex_presentation = ip;
    queueFamilyIndex_compute = ic;
    return VK_SUCCESS;
  }

  VkResult CreateDebugMessenger() {
    constexpr static const PFN_vkDebugUtilsMessengerCallbackEXT
        DebugUtilsMessengerCallback =
            [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
               VkDebugUtilsMessageTypeFlagsEXT messageTypes,
               const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
               void *pUserData) -> VkBool32 {
      printf("[ graphicsBase ] DEBUG\n%s\n", pCallbackData->pMessage);
      return VK_FALSE;
    };

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = DebugUtilsMessengerCallback};
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (vkCreateDebugUtilsMessenger) {
      VkResult result = vkCreateDebugUtilsMessenger(
          instance, &debugUtilsMessengerCreateInfo, nullptr, &debugMessenger);
      if (result)
        printf("[ graphicsBase ] ERROR: Failed to create a "
               "debug messenger!\nError code: %d\n",
               int32_t(result));
      return result;
    }
    printf("[ graphicsBase ] ERROR: Failed to get the function pointer of "
           "vkCreateDebugUtilsMessengerEXT!\n");
    return VK_RESULT_MAX_ENUM;
  }

public:
  // Getter
  uint32_t ApiVersion() const { return apiVersion; }
  VkInstance Instance() const { return instance; }
  VkPhysicalDevice PhysicalDevice() const { return physicalDevice; }
  const VkPhysicalDeviceProperties &PhysicalDeviceProperties() const {
    return physicalDeviceProperties;
  }
  const VkPhysicalDeviceMemoryProperties &
  PhysicalDeviceMemoryProperties() const {
    return physicalDeviceMemoryProperties;
  }
  VkPhysicalDevice AvailablePhysicalDevice(uint32_t index) const {
    return availablePhysicalDevices[index];
  }
  uint32_t AvailablePhysicalDeviceCount() const {
    return uint32_t(availablePhysicalDevices.size());
  }

  VkDevice Device() const { return device; }
  uint32_t QueueFamilyIndex_Graphics() const {
    return queueFamilyIndex_graphics;
  }
  uint32_t QueueFamilyIndex_Presentation() const {
    return queueFamilyIndex_presentation;
  }
  uint32_t QueueFamilyIndex_Compute() const { return queueFamilyIndex_compute; }
  VkQueue Queue_Graphics() const { return queue_graphics; }
  VkQueue Queue_Presentation() const { return queue_presentation; }
  VkQueue Queue_Compute() const { return queue_compute; }

  VkSurfaceKHR Surface() const { return surface; }
  const VkFormat &AvailableSurfaceFormat(uint32_t index) const {
    return availableSurfaceFormats[index].format;
  }
  const VkColorSpaceKHR &AvailableSurfaceColorSpace(uint32_t index) const {
    return availableSurfaceFormats[index].colorSpace;
  }
  uint32_t AvailableSurfaceFormatCount() const {
    return uint32_t(availableSurfaceFormats.size());
  }

  VkSwapchainKHR Swapchain() const { return swapchain; }
  VkImage SwapchainImage(uint32_t index) const {
    return swapchainImages[index];
  }
  VkImageView SwapchainImageView(uint32_t index) const {
    return swapchainImageViews[index];
  }
  uint32_t SwapchainImageCount() const {
    return uint32_t(swapchainImages.size());
  }
  const VkSwapchainCreateInfoKHR &SwapchainCreateInfo() const {
    return swapchainCreateInfo;
  }

  const std::vector<const char *> &InstanceLayers() const {
    return instanceLayers;
  }
  const std::vector<const char *> &InstanceExtensions() const {
    return instanceExtensions;
  }
  const std::vector<const char *> &DeviceExtensions() const {
    return deviceExtensions;
  }

  // Const函数
  VkResult CheckInstanceLayers(const char **layersToCheck, int length) const {
    uint32_t layerCount;
    std::vector<VkLayerProperties> availableLayers;
    VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
      printf("[ graphicsBase ] ERROR: Failed to get the "
             "count of instance layers!\n");
      return result;
    }
    if (result == VK_INCOMPLETE) {
      printf("[ graphicsBase ] WARNING: The count of instance layers is "
             "incomplete!\n");
    }
    if (layerCount) {
      availableLayers.resize(layerCount);
      if (VkResult result = vkEnumerateInstanceLayerProperties(
              &layerCount, availableLayers.data())) {
        printf("[ graphicsBase ] ERROR: Failed to enumerate "
               "instance layer properties!\nError code: %d\n",
               int32_t(result));
        return result;
      }
      for (int i = 0; i < length; i++) {
        auto &str1 = layersToCheck[i];
        bool found = false;
        for (auto &str2 : availableLayers)
          if (!strcmp(str1, str2.layerName)) {
            found = true;
            break;
          }
        if (!found)
          str1 = nullptr;
      }
    } else
      for (int i = 0; i < length; i++) {
        layersToCheck[i] = nullptr;
      }
    // 一切顺利则返回VK_SUCCESS
    return VK_SUCCESS;
  }

  VkResult CheckInstanceExtensions(const char **extensionsToCheck, int length,
                                   const char *layerName = nullptr) const {
    uint32_t extensionCount;
    std::vector<VkExtensionProperties> availableExtensions;
    VkResult result = vkEnumerateInstanceExtensionProperties(
        layerName, &extensionCount, nullptr);
    if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
      layerName
          ? printf(
                "[ graphicsBase ] ERROR: Failed to get the count of instance "
                "extensions!\nLayer name:%s\n",
                layerName)
          : printf("[ graphicsBase ] ERROR: Failed to get "
                   "the count of instance extensions!\n");
      return result;
    }
    if (result == VK_INCOMPLETE) {
      layerName
          ? printf(
                "[ graphicsBase ] WARNING: The count of instance extensions is "
                "incomplete!\nLayer name:%s\n",
                layerName)
          : printf("[ graphicsBase ] WARNING: The count of instance extensions "
                   "is incomplete!\n");
    }

    if (extensionCount) {
      availableExtensions.resize(extensionCount);
      if (VkResult result = vkEnumerateInstanceExtensionProperties(
              layerName, &extensionCount, availableExtensions.data())) {
        printf("[ graphicsBase ] ERROR: Failed to enumerate instance extension "
               "properties!\nError code: %d\n",
               int32_t(result));
        return result;
      }
      for (int i = 0; i < length; i++) {
        auto &str1 = extensionsToCheck[i];
        bool found = false;
        for (auto &str2 : availableExtensions)
          if (!strcmp(str1, str2.extensionName)) {
            found = true;
            break;
          }
        if (!found)
          str1 = nullptr;
      }
    } else
      for (int i = 0; i < length; i++) {
        auto &str1 = extensionsToCheck[i];
        str1 = nullptr;
      }
    return VK_SUCCESS;
  }
  VkResult CheckDeviceExtensions(const char *extensionsToCheck, int length,
                                 const char *layerName = nullptr) const {
    /*待Ch1-3填充*/
  }

  // Non-const函数
  void AddInstanceLayer(const char *layerName) {
    instanceLayers.push_back(layerName);
  }
  void AddInstanceExtension(const char *extensionName) {
    instanceExtensions.push_back(extensionName);
  }
  void AddDeviceExtension(const char *extensionName) {
    deviceExtensions.push_back(extensionName);
  }

  VkResult UseLatestApiVersion() {
    if (vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion")) {
      return vkEnumerateInstanceVersion(&apiVersion);
    }
    return VK_SUCCESS;
  }

  VkResult CreateInstance(VkInstanceCreateFlags flags = 0) {
#ifndef NDEBUG
    AddInstanceLayer("VK_LAYER_KHRONOS_validation");
    AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    VkApplicationInfo applicatianInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = apiVersion};
    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = flags,
        .pApplicationInfo = &applicatianInfo,
        .enabledLayerCount = uint32_t(instanceLayers.size()),
        .ppEnabledLayerNames = instanceLayers.data(),
        .enabledExtensionCount = uint32_t(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data()};
    if (VkResult result =
            vkCreateInstance(&instanceCreateInfo, nullptr, &instance)) {
      printf("[ graphicsBase ] ERROR: Failed to create a "
             "vulkan instance!\nError code: %d\n",
             int32_t(result));
      return result;
    }
    // 成功创建Vulkan实例后，输出Vulkan版本
    printf("[ graphicsBase ] Vulkan API Version: %d.%d.%d\n",
           uint32_t(VK_VERSION_MAJOR(apiVersion)),
           uint32_t(VK_VERSION_MINOR(apiVersion)),
           uint32_t(VK_VERSION_PATCH(apiVersion)));
#ifndef NDEBUG
    // 创建完Vulkan实例后紧接着创建debug messenger
    CreateDebugMessenger();
#endif
    return VK_SUCCESS;
  }
  void Surface(VkSurfaceKHR surface) {
    if (!this->surface)
      this->surface = surface;
  }
  VkResult GetPhysicalDevices() {
    uint32_t deviceCount;
    if (VkResult result =
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr)) {
      printf("[ graphicsBase ] ERROR: Failed to get the "
             "count of physical devices!\nError code: %d\n",
             int32_t(result));
      return result;
    }
    if (!deviceCount) {
      printf("[ graphicsBase ] ERROR: Failed to find any "
             "physical device supports vulkan!\n"),
          abort();
    }
    availablePhysicalDevices.resize(deviceCount);
    VkResult result = vkEnumeratePhysicalDevices(
        instance, &deviceCount, availablePhysicalDevices.data());
    if (result)
      printf("[ graphicsBase ] ERROR: Failed to enumerate "
             "physical devices!\nError code: %d\n",
             int32_t(result));
    return result;
  }
  VkResult DeterminePhysicalDevice(uint32_t deviceIndex,
                                   bool enableGraphicsQueue,
                                   bool enableComputeQueue) {
    // 定义一个特殊值用于标记一个队列族索引已被找过但未找到
    static constexpr uint32_t notFound =
        INT32_MAX; //== VK_QUEUE_FAMILY_IGNORED & INT32_MAX
    // 定义队列族索引组合的结构体
    struct queueFamilyIndexCombination {
      uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
      uint32_t presentation = VK_QUEUE_FAMILY_IGNORED;
      uint32_t compute = VK_QUEUE_FAMILY_IGNORED;
    };
    // queueFamilyIndexCombinations用于为各个物理设备保存一份队列族索引组合
    static std::vector<queueFamilyIndexCombination>
        queueFamilyIndexCombinations(availablePhysicalDevices.size());
    auto &[ig, ip, ic] = queueFamilyIndexCombinations[deviceIndex];

    // 如果有任何队列族索引已被找过但未找到，返回VK_RESULT_MAX_ENUM
    if (ig == notFound && enableGraphicsQueue || ip == notFound && surface ||
        ic == notFound && enableComputeQueue)
      return VK_RESULT_MAX_ENUM;

    // 如果有任何队列族索引应被获取但还未被找过
    if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphicsQueue ||
        ip == VK_QUEUE_FAMILY_IGNORED && surface ||
        ic == VK_QUEUE_FAMILY_IGNORED && enableComputeQueue) {
      uint32_t indices[3];
      VkResult result = GetQueueFamilyIndices(
          availablePhysicalDevices[deviceIndex], enableGraphicsQueue,
          enableComputeQueue, indices);
      // 若GetQueueFamilyIndices(...)返回VK_SUCCESS或VK_RESULT_MAX_ENUM（vkGetPhysicalDeviceSurfaceSupportKHR(...)执行成功但没找齐所需队列族），
      // 说明对所需队列族索引已有结论，保存结果到queueFamilyIndexCombinations[deviceIndex]中相应变量
      // 应被获取的索引若仍为VK_QUEUE_FAMILY_IGNORED，说明未找到相应队列族，VK_QUEUE_FAMILY_IGNORED（~0u）与INT32_MAX做位与得到的数值等于notFound
      if (result == VK_SUCCESS || result == VK_RESULT_MAX_ENUM) {
        if (enableGraphicsQueue)
          ig = indices[0] & INT32_MAX;
        if (surface)
          ip = indices[1] & INT32_MAX;
        if (enableComputeQueue)
          ic = indices[2] & INT32_MAX;
      }
      // 如果GetQueueFamilyIndices(...)执行失败，return
      if (result)
        return result;
    }

    // 若以上两个if分支皆不执行，则说明所需的队列族索引皆已被获取，从queueFamilyIndexCombinations[deviceIndex]中取得索引
    else {
      queueFamilyIndex_graphics =
          enableGraphicsQueue ? ig : VK_QUEUE_FAMILY_IGNORED;
      queueFamilyIndex_presentation = surface ? ip : VK_QUEUE_FAMILY_IGNORED;
      queueFamilyIndex_compute =
          enableComputeQueue ? ic : VK_QUEUE_FAMILY_IGNORED;
    }
    physicalDevice = availablePhysicalDevices[deviceIndex];
    return VK_SUCCESS;
  }
  VkResult CreateDevice(VkDeviceCreateFlags flags = 0) {
    float queuePriority = 1.f;
    VkDeviceQueueCreateInfo queueCreateInfos[3] = {
        {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .queueCount = 1,
         .pQueuePriorities = &queuePriority},
        {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .queueCount = 1,
         .pQueuePriorities = &queuePriority},
        {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .queueCount = 1,
         .pQueuePriorities = &queuePriority}};
    uint32_t queueCreateInfoCount = 0;
    if (queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED)
      queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex =
          queueFamilyIndex_graphics;
    if (queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED &&
        queueFamilyIndex_presentation != queueFamilyIndex_graphics)
      queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex =
          queueFamilyIndex_presentation;
    if (queueFamilyIndex_compute != VK_QUEUE_FAMILY_IGNORED &&
        queueFamilyIndex_compute != queueFamilyIndex_graphics &&
        queueFamilyIndex_compute != queueFamilyIndex_presentation)
      queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex =
          queueFamilyIndex_compute;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .flags = flags,
        .queueCreateInfoCount = queueCreateInfoCount,
        .pQueueCreateInfos = queueCreateInfos,
        .enabledExtensionCount = uint32_t(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &physicalDeviceFeatures};
    if (VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo,
                                         nullptr, &device)) {
      printf("[ graphicsBase ] ERROR: Failed to create a "
             "vulkan logical device!\nError code: %d\n",
             int32_t(result));
      return result;
    }
    if (queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED)
      vkGetDeviceQueue(device, queueFamilyIndex_graphics, 0, &queue_graphics);
    if (queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED)
      vkGetDeviceQueue(device, queueFamilyIndex_presentation, 0,
                       &queue_presentation);
    if (queueFamilyIndex_compute != VK_QUEUE_FAMILY_IGNORED)
      vkGetDeviceQueue(device, queueFamilyIndex_compute, 0, &queue_compute);
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice,
                                        &physicalDeviceMemoryProperties);
    // 输出所用的物理设备名称
    printf("[ graphicsBase ] INFO: Renderer: %s\n",
           physicalDeviceProperties.deviceName);
    printf("[ graphicsBase ] INFO: Renderer type: %d\n",
           physicalDeviceProperties.deviceType);
    printf("[ graphicsBase ] INFO: Renderer API version: %d.%d.%d\n",
           uint32_t(VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion)),
           uint32_t(VK_VERSION_MINOR(physicalDeviceProperties.apiVersion)),
           uint32_t(VK_VERSION_PATCH(physicalDeviceProperties.apiVersion)));
    printf("[ graphicsBase ] INFO: Renderer driver version: %d\n",
           physicalDeviceProperties.driverVersion);
    return VK_SUCCESS;
  }
  VkResult GetSurfaceFormats() {
    uint32_t surfaceFormatCount;
    if (VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice, surface, &surfaceFormatCount, nullptr)) {
      printf("[ graphicsBase ] ERROR\nFailed to get the "
             "count of surface formats!\nError code: %d\n",
             int32_t(result));
      return result;
    }
    if (!surfaceFormatCount)
      printf("[ graphicsBase ] ERROR\nFailed to find any "
             "supported surface format!\n"),
          abort();
    availableSurfaceFormats.resize(surfaceFormatCount);
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice, surface, &surfaceFormatCount,
        availableSurfaceFormats.data());
    if (result)
      printf("[ graphicsBase ] ERROR\nFailed to get surface "
             "formats!\nError code: %d\n",
             int32_t(result));
    return result;
  }
  VkResult SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat) {

    bool formatIsAvailable = false;
    if (!surfaceFormat.format) {
      // 如果格式未指定，只匹配色彩空间，图像格式有啥就用啥
      for (auto &i : availableSurfaceFormats)
        if (i.colorSpace == surfaceFormat.colorSpace) {
          swapchainCreateInfo.imageFormat = i.format;
          swapchainCreateInfo.imageColorSpace = i.colorSpace;
          formatIsAvailable = true;
          break;
        }
    } else
      // 否则匹配格式和色彩空间
      for (auto &i : availableSurfaceFormats)
        if (i.format == surfaceFormat.format &&
            i.colorSpace == surfaceFormat.colorSpace) {
          swapchainCreateInfo.imageFormat = i.format;
          swapchainCreateInfo.imageColorSpace = i.colorSpace;
          formatIsAvailable = true;
          break;
        }
    // 如果没有符合的格式，恰好有个语义相符的错误代码
    if (!formatIsAvailable)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;
    // 如果交换链已存在，调用RecreateSwapchain()重建交换链
    if (swapchain)
      return RecreateSwapchain();
    return VK_SUCCESS;
  }
  VkResult CreateSwapchain(bool limitFrameRate = true,
                           VkSwapchainCreateFlagsKHR flags = 0) {
    /*待Ch1-4填充*/
  }

  VkResult RecreateSwapchain() { /*待Ch1-4填充*/ }

  void InstanceLayers(const std::vector<const char *> &layerNames) {
    instanceLayers = layerNames;
  }
  void InstanceExtensions(const std::vector<const char *> &extensionNames) {
    instanceExtensions = extensionNames;
  }
  void DeviceExtensions(const std::vector<const char *> &extensionNames) {
    deviceExtensions = extensionNames;
  }

  // 单例
  static graphicsBase &Singleton() {
    static graphicsBase singleton;
    return singleton;
  }
};
