#include "imageMemory.hpp"

#include "../Models/graphic.hpp"

#include <utility>

vulkanWrapper::imageMemory::imageMemory() = default;
vulkanWrapper::imageMemory::imageMemory(
    VkImageCreateInfo &createInfo,
    VkMemoryPropertyFlags desiredMemoryProperties) {
  Create(createInfo, desiredMemoryProperties);
}
vulkanWrapper::imageMemory::imageMemory(imageMemory &&other) noexcept
    : image(std::move(other)), deviceMemory(std::move(other)) {
  areBound = other.areBound;
  other.areBound = false;
}
vulkanWrapper::imageMemory::~imageMemory() { areBound = false; }

// Non-const Function
// 以下三个函数仅用于Create(...)可能执行失败的情况
VkResultThrowable
vulkanWrapper::imageMemory::CreateImage(VkImageCreateInfo &createInfo) {
  return image::Create(createInfo);
}
VkResultThrowable vulkanWrapper::imageMemory::AllocateMemory(
    VkMemoryPropertyFlags desiredMemoryProperties) {
  VkMemoryAllocateInfo allocateInfo =
      MemoryAllocateInfo(desiredMemoryProperties);
  if (allocateInfo.memoryTypeIndex >=
      graphic::Singleton().PhysicalDeviceMemoryProperties().memoryTypeCount)
    return VK_RESULT_MAX_ENUM; // 没有合适的错误代码，别用VK_ERROR_UNKNOWN
  return Allocate(allocateInfo);
}
VkResultThrowable vulkanWrapper::imageMemory::BindMemory() {
  if (VkResult result = image::BindMemory(Memory()))
    return result;
  areBound = true;
  return VK_SUCCESS;
}
// 分配设备内存、创建图像、绑定
VkResultThrowable vulkanWrapper::imageMemory::Create(
    VkImageCreateInfo &createInfo,
    VkMemoryPropertyFlags desiredMemoryProperties) {
  VkResult result;
  false || // 这行用来应对Visual Studio中代码的对齐
      (result = CreateImage(createInfo)) || // 用||短路执行
      (result = AllocateMemory(desiredMemoryProperties)) ||
      (result = BindMemory());
  return result;
}
