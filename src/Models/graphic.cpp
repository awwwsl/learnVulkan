#define GLFW_INCLUDE_VULKAN

#include "graphic.hpp"
#include "../Utils/VkResultThrowable.hpp"

#include <glm/glm.hpp>
#include <stdio.h>
#include <string.h>

graphicsBase::~graphicsBase() {
  if (!instance)
    return;
  if (device) {
    WaitIdle();
    if (swapchain) {
      for (auto &i : destroySwapchainCallbacks)
        i();
      for (auto &i : swapchainImageViews)
        if (i)
          vkDestroyImageView(device, i, nullptr);
      vkDestroySwapchainKHR(device, swapchain, nullptr);
    }
    for (auto &i : destroyDeviceCallbacks)
      i();
    vkDestroyDevice(device, nullptr);
  }
  if (surface)
    vkDestroySurfaceKHR(instance, surface, nullptr);
  if (debugMessenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessenger =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (DestroyDebugUtilsMessenger)
      DestroyDebugUtilsMessenger(instance, debugMessenger, nullptr);
  }
  vkDestroyInstance(instance, nullptr);
}

VkResultThrowable graphicsBase::GetQueueFamilyIndices(
    VkPhysicalDevice physicalDevice, bool enableGraphicsQueue,
    bool enableComputeQueue, uint32_t (&queueFamilyIndices)[3]) {
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
      if (VkResultThrowable result = vkGetPhysicalDeviceSurfaceSupportKHR(
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

VkResultThrowable graphicsBase::CreateDebugMessenger() {
  constexpr static const PFN_vkDebugUtilsMessengerCallbackEXT
      DebugUtilsMessengerCallback =
          [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
             VkDebugUtilsMessageTypeFlagsEXT messageTypes,
             const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
             void *pUserData) -> VkBool32 {
    printf("[ graphicsBase ] DEBUG: %s\n", pCallbackData->pMessage);
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
    VkResultThrowable result = vkCreateDebugUtilsMessenger(
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
VkResultThrowable graphicsBase::CheckInstanceLayers(const char **layersToCheck,
                                                    int length) const {
  uint32_t layerCount;
  std::vector<VkLayerProperties> availableLayers;
  VkResultThrowable result =
      vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
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
    if (VkResultThrowable result = vkEnumerateInstanceLayerProperties(
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

VkResultThrowable
graphicsBase::CheckInstanceExtensions(const char **extensionsToCheck,
                                      int length, const char *layerName) const {
  uint32_t extensionCount;
  std::vector<VkExtensionProperties> availableExtensions;
  VkResultThrowable result = vkEnumerateInstanceExtensionProperties(
      layerName, &extensionCount, nullptr);
  if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
    layerName
        ? printf("[ graphicsBase ] ERROR: Failed to get the count of instance "
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
    if (VkResultThrowable result = vkEnumerateInstanceExtensionProperties(
            layerName, &extensionCount, availableExtensions.data())) {
      printf("[ graphicsBase ] ERROR: Failed to enumerate instance extension "
             "properties!\nError code: %d\n",
             int32_t(result));
      return result;
    }
    for (int i = 0; i < length; i++) {
      const char *&str1 = extensionsToCheck[i];
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
      const char *&str1 = extensionsToCheck[i];
      str1 = nullptr;
    }
  return VK_SUCCESS;
}

VkResultThrowable
graphicsBase::CheckDeviceExtensions(const char **extensionsToCheck, int length,
                                    const char *layerName) const {
  uint32_t extensionCount;
  std::vector<VkExtensionProperties> availableExtensions;
  if (VkResultThrowable result = vkEnumerateDeviceExtensionProperties(
          physicalDevice, layerName, &extensionCount, nullptr)) {
    layerName
        ? printf("[ graphicsBase ] ERROR: Failed to get the count of device"
                 "extensions!\nLayer name: %s\n",
                 layerName)
        : printf("[ graphicsBase ] ERROR: Failed to get "
                 "the count of device extensions!\n");
    return result;
  }
  if (extensionCount) {
    availableExtensions.resize(extensionCount);
    if (VkResultThrowable result = vkEnumerateDeviceExtensionProperties(
            physicalDevice, layerName, &extensionCount,
            availableExtensions.data())) {
      printf("[ graphicsBase ] ERROR: Failed to enumerate device extension "
             "properties!\nError code: %d\n",
             int32_t(result));
      return result;
    }
    for (int i = 0; i < length; i++) {
      // for (auto &i : extensionsToCheck) {
      const char *str1 = extensionsToCheck[i];
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
      extensionsToCheck[i] = nullptr;
    }
  return VK_SUCCESS;
}

VkResultThrowable graphicsBase::CreateInstance(VkInstanceCreateFlags flags) {
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
  if (VkResultThrowable result =
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

VkResultThrowable graphicsBase::GetPhysicalDevices() {
  uint32_t deviceCount;
  if (VkResultThrowable result =
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
  VkResultThrowable result = vkEnumeratePhysicalDevices(
      instance, &deviceCount, availablePhysicalDevices.data());
  if (result)
    printf("[ graphicsBase ] ERROR: Failed to enumerate "
           "physical devices!\nError code: %d\n",
           int32_t(result));
  return result;
}

VkResultThrowable graphicsBase::DeterminePhysicalDevice(
    uint32_t deviceIndex, bool enableGraphicsQueue, bool enableComputeQueue) {
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
  static std::vector<queueFamilyIndexCombination> queueFamilyIndexCombinations(
      availablePhysicalDevices.size());
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
    VkResultThrowable result =
        GetQueueFamilyIndices(availablePhysicalDevices[deviceIndex],
                              enableGraphicsQueue, enableComputeQueue, indices);
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

VkResultThrowable graphicsBase::CreateDevice(VkDeviceCreateFlags flags) {
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
  if (VkResultThrowable result =
          vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device)) {
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
  printf("[ graphicsBase ] INFO: Renderer driver version: %u\n",
         physicalDeviceProperties.driverVersion);

  for (auto &i : createDeviceCallbacks)
    i();
  return VK_SUCCESS;
}

VkResultThrowable graphicsBase::GetSurfaceFormats() {
  uint32_t surfaceFormatCount;
  if (VkResultThrowable result = vkGetPhysicalDeviceSurfaceFormatsKHR(
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
  VkResultThrowable result = vkGetPhysicalDeviceSurfaceFormatsKHR(
      physicalDevice, surface, &surfaceFormatCount,
      availableSurfaceFormats.data());
  if (result)
    printf("[ graphicsBase ] ERROR\nFailed to get surface "
           "formats!\nError code: %d\n",
           int32_t(result));
  return result;
}

VkResultThrowable
graphicsBase::SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat) {

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

VkResultThrowable graphicsBase::CreateSwapchain_Internal() {
  if (VkResultThrowable result = vkCreateSwapchainKHR(
          device, &swapchainCreateInfo, nullptr, &swapchain)) {
    printf("[ graphicsBase ] ERROR: Failed to create a "
           "swapchain!\nError code: %d\n",
           int32_t(result));
    return result;
  }

  // 获取交换连图像
  uint32_t swapchainImageCount;
  if (VkResultThrowable result = vkGetSwapchainImagesKHR(
          device, swapchain, &swapchainImageCount, nullptr)) {
    printf("[ graphicsBase ] ERROR: Failed to get the "
           "count of swapchain images!\nError code: %d\n",
           int32_t(result));
    return result;
  }
  swapchainImages.resize(swapchainImageCount);
  if (VkResultThrowable result = vkGetSwapchainImagesKHR(
          device, swapchain, &swapchainImageCount, swapchainImages.data())) {
    printf("[ graphicsBase ] ERROR: Failed to get "
           "swapchain images!\nError code: %d\n",
           int32_t(result));
    return result;
  }

  // 创建image view
  swapchainImageViews.resize(swapchainImageCount);
  VkImageViewCreateInfo imageViewCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = swapchainCreateInfo.imageFormat,
      //.components = {},//四个成员皆为VK_COMPONENT_SWIZZLE_IDENTITY
      .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
  for (size_t i = 0; i < swapchainImageCount; i++) {
    imageViewCreateInfo.image = swapchainImages[i];
    if (VkResultThrowable result = vkCreateImageView(
            device, &imageViewCreateInfo, nullptr, &swapchainImageViews[i])) {
      printf("[ graphicsBase ] ERROR: Failed to create a "
             "swapchain image view!\nError code: %d\n",
             int32_t(result));
      return result;
    }
  }
  return VK_SUCCESS;
}

VkResultThrowable
graphicsBase::CreateSwapchain(bool limitFrameRate,
                              VkSwapchainCreateFlagsKHR flags) {
  // VkSurfaceCapabilitiesKHR相关的参数
  VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
  if (VkResultThrowable result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          physicalDevice, surface, &surfaceCapabilities)) {
    printf("[ graphicsBase ] ERROR: Failed to get physical "
           "device surface capabilities!\nError code: %d\n",
           int32_t(result));
    return result;
  }
  // 指定图像数量
  swapchainCreateInfo.minImageCount =
      surfaceCapabilities.minImageCount +
      (surfaceCapabilities.maxImageCount > surfaceCapabilities.minImageCount);
  // 指定图像大小
  swapchainCreateInfo.imageExtent =
      surfaceCapabilities.currentExtent.width == -1
          ? VkExtent2D{glm::clamp(defaultSize.width,
                                  surfaceCapabilities.minImageExtent.width,
                                  surfaceCapabilities.maxImageExtent.width),
                       glm::clamp(defaultSize.height,
                                  surfaceCapabilities.minImageExtent.height,
                                  surfaceCapabilities.maxImageExtent.height)}
          : surfaceCapabilities.currentExtent;
  // swapchainCreateInfo.imageArrayLayers =
  // 1;//跟其他不需要判断的参数一起扔后面去 指定变换方式
  swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
  // 指定处理透明通道的方式
  if (surfaceCapabilities.supportedCompositeAlpha &
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
  else
    for (size_t i = 0; i < 4; i++)
      if (surfaceCapabilities.supportedCompositeAlpha & 1 << i) {
        swapchainCreateInfo.compositeAlpha = VkCompositeAlphaFlagBitsKHR(
            surfaceCapabilities.supportedCompositeAlpha & 1 << i);
        break;
      }
  // 指定图像用途
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
    swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  else
    printf("[ graphicsBase ] WARNING: VK_IMAGE_USAGE_TRANSFER_DST_BIT isn't "
           "supported!\n");

  // 指定图像格式
  if (!availableSurfaceFormats.size())
    if (VkResultThrowable result = GetSurfaceFormats())
      return result;
  if (!swapchainCreateInfo.imageFormat)
    // 用&&操作符来短路执行
    if (SetSurfaceFormat(
            {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}) &&
        SetSurfaceFormat(
            {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})) {
      // 如果找不到上述图像格式和色彩空间的组合，那只能有什么用什么，采用availableSurfaceFormats中的第一组
      swapchainCreateInfo.imageFormat = availableSurfaceFormats[0].format;
      swapchainCreateInfo.imageColorSpace =
          availableSurfaceFormats[0].colorSpace;
      printf("[ graphicsBase ] WARNING: Failed to select a "
             "four-component UNORM surface format!\n");
    }

  // 指定呈现模式
  uint32_t surfacePresentModeCount;
  if (VkResultThrowable result = vkGetPhysicalDeviceSurfacePresentModesKHR(
          physicalDevice, surface, &surfacePresentModeCount, nullptr)) {
    printf("[ graphicsBase ] ERROR: Failed to get the count of surface "
           "present modes!\nError code: %d\n",
           int32_t(result));
    return result;
  }
  if (!surfacePresentModeCount)
    printf("[ graphicsBase ] ERROR: Failed to find any "
           "surface present mode!\n"),
        abort();
  std::vector<VkPresentModeKHR> surfacePresentModes(surfacePresentModeCount);
  if (VkResultThrowable result = vkGetPhysicalDeviceSurfacePresentModesKHR(
          physicalDevice, surface, &surfacePresentModeCount,
          surfacePresentModes.data())) {
    printf("[ graphicsBase ] ERROR: Failed to get "
           "surface present modes!\nError code: %d\n",
           int32_t(result));
    return result;
  }
  swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  if (!limitFrameRate)
    for (size_t i = 0; i < surfacePresentModeCount; i++)
      if (surfacePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
        swapchainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        break;
      }

  // 剩余参数
  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.flags = flags;
  swapchainCreateInfo.surface = surface;
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchainCreateInfo.clipped = VK_TRUE;

  // 创建交换链
  if (VkResultThrowable result = CreateSwapchain_Internal())
    return result;
  // 执行回调函数
  for (auto &i : createSwapchainCallbacks)
    i();
  return VK_SUCCESS;
}

VkResultThrowable graphicsBase::RecreateSwapchain() {
  VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
  if (VkResultThrowable result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          physicalDevice, surface, &surfaceCapabilities)) {
    printf("[ graphicsBase ] ERROR: Failed to get physical device surface "
           "capabilities!\nError code: %d\n",
           int32_t(result));
    return result;
  }
  if (surfaceCapabilities.currentExtent.width == 0 ||
      surfaceCapabilities.currentExtent.height == 0)
    return VK_SUBOPTIMAL_KHR;
  swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
  swapchainCreateInfo.oldSwapchain = swapchain;
  VkResultThrowable result = vkQueueWaitIdle(queue_graphics);
  if (result == VK_SUCCESS && queue_graphics != queue_presentation)
    result = vkQueueWaitIdle(queue_presentation);
  if (result) {
    printf("[ graphicsBase ] ERROR: Failed to wait for "
           "the queue to be idle!\nError code: %d\n",
           int32_t(result));
    return result;
  }
  // 销毁旧交换链相关对象
  for (auto &i : destroySwapchainCallbacks)
    i();
  for (auto &i : swapchainImageViews)
    if (i)
      vkDestroyImageView(device, i, nullptr);
  swapchainImageViews.resize(0);
  // 创建新交换链及与之相关的对象
  if (VkResultThrowable result = CreateSwapchain_Internal())
    return result;
  for (auto &i : createSwapchainCallbacks)
    i();
  return VK_SUCCESS;
}

// 该函数用于等待逻辑设备空闲
VkResultThrowable graphicsBase::WaitIdle() const {
  VkResultThrowable result = vkDeviceWaitIdle(device);
  if (result)
    printf("[ graphicsBase ] ERROR: Failed to wait for the "
           "device to be idle!\nError code: %d\n",
           int32_t(result));
  return result;
}
// 该函数用于重建逻辑设备
VkResultThrowable graphicsBase::RecreateDevice(VkDeviceCreateFlags flags) {
  if (VkResultThrowable result = WaitIdle())
    return result;
  if (swapchain) {
    for (auto &i : destroySwapchainCallbacks)
      i();
    for (auto &i : swapchainImageViews)
      if (i)
        vkDestroyImageView(device, i, nullptr);
    swapchainImageViews.resize(0);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    swapchain = VK_NULL_HANDLE;
    swapchainCreateInfo = {};
  }
  for (auto &i : destroySwapchainCallbacks)
    i();
  if (device)
    vkDestroyDevice(device, nullptr), device = VK_NULL_HANDLE;
  return CreateDevice(flags);
}

void graphicsBase::Terminate() {
  this->~graphicsBase();
  instance = VK_NULL_HANDLE;
  physicalDevice = VK_NULL_HANDLE;
  device = VK_NULL_HANDLE;
  surface = VK_NULL_HANDLE;
  swapchain = VK_NULL_HANDLE;
  swapchainImages.resize(0);
  swapchainImageViews.resize(0);
  swapchainCreateInfo = {};
  debugMessenger = VK_NULL_HANDLE;
}

// 单例
graphicsBase &graphicsBase::Singleton() {
  static graphicsBase singleton;
  return singleton;
}
