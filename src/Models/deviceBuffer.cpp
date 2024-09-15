#include "graphic.hpp"

#include "deviceBuffer.hpp"

vulkanWrapper::bufferMemory::bufferMemory() = default;
vulkanWrapper::bufferMemory::bufferMemory(
    VkBufferCreateInfo &createInfo,
    VkMemoryPropertyFlags desiredMemoryProperties) {
  Create(createInfo, desiredMemoryProperties);
}
vulkanWrapper::bufferMemory::bufferMemory(bufferMemory &&other) noexcept
    : buffer(std::move(other)), deviceMemory(std::move(other)) {
  areBound = other.areBound;
  other.areBound = false;
}
vulkanWrapper::bufferMemory::~bufferMemory() { areBound = false; }

// Non-const Function
// 以下三个函数仅用于Create(...)可能执行失败的情况
VkResultThrowable
vulkanWrapper::bufferMemory::CreateBuffer(VkBufferCreateInfo &createInfo) {
  return buffer::Create(createInfo);
}
VkResultThrowable vulkanWrapper::bufferMemory::AllocateMemory(
    VkMemoryPropertyFlags desiredMemoryProperties) {
  VkMemoryAllocateInfo allocateInfo =
      MemoryAllocateInfo(desiredMemoryProperties);
  if (allocateInfo.memoryTypeIndex >=
      graphic::Singleton().PhysicalDeviceMemoryProperties().memoryTypeCount)
    return VK_RESULT_MAX_ENUM; // 没有合适的错误代码，别用VK_ERROR_UNKNOWN
  return Allocate(allocateInfo);
}
VkResultThrowable vulkanWrapper::bufferMemory::BindMemory() {
  if (VkResult result = buffer::BindMemory(Memory()))
    return result;
  areBound = true;
  return VK_SUCCESS;
}
// 分配设备内存、创建缓冲、绑定
VkResultThrowable vulkanWrapper::bufferMemory::Create(
    VkBufferCreateInfo &createInfo,
    VkMemoryPropertyFlags desiredMemoryProperties) {
  VkResult result;
  false || // 这行用来应对Visual Studio中代码的对齐
      (result = CreateBuffer(createInfo)) || // 用||短路执行
      (result = AllocateMemory(desiredMemoryProperties)) ||
      (result = BindMemory());
  return result;
}
