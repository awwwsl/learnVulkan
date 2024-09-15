#include "deviceMemory.hpp"

#include "../Models/graphic.hpp"

#include <cstring>

VkDeviceSize vulkanWrapper::deviceMemory::AdjustNonCoherentMemoryRange(
    VkDeviceSize &size, VkDeviceSize &offset) const {
  const VkDeviceSize &nonCoherentAtomSize = graphic::Singleton()
                                                .PhysicalDeviceProperties()
                                                .limits.nonCoherentAtomSize;
  VkDeviceSize _offset = offset;
  offset = offset / nonCoherentAtomSize * nonCoherentAtomSize;
  size = std::min((size + _offset + nonCoherentAtomSize - 1) /
                      nonCoherentAtomSize * nonCoherentAtomSize,
                  allocationSize) -
         offset;
  return _offset - offset;
}

vulkanWrapper::deviceMemory::deviceMemory() = default;
vulkanWrapper::deviceMemory::deviceMemory(VkMemoryAllocateInfo &allocateInfo) {
  Allocate(allocateInfo);
}
vulkanWrapper::deviceMemory::deviceMemory(deviceMemory &&other) noexcept {
  MoveHandle;
  allocationSize = other.allocationSize;
  memoryProperties = other.memoryProperties;
  other.allocationSize = 0;
  other.memoryProperties = 0;
}
vulkanWrapper::deviceMemory::~deviceMemory() {
  DestroyHandleBy(vkFreeMemory, "deviceMemory") allocationSize = 0;
  memoryProperties = 0;
}
// Const Function
// 映射host visible的内存区
VkResultThrowable
vulkanWrapper::deviceMemory::MapMemory(void *&pData, VkDeviceSize size,
                                       VkDeviceSize offset) const {
  VkDeviceSize inverseDeltaOffset;
  if (!(memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    inverseDeltaOffset = AdjustNonCoherentMemoryRange(size, offset);
  if (VkResult result = vkMapMemory(graphic::Singleton().Device(), handle,
                                    offset, size, 0, &pData)) {
    printf(
        "[ deviceMemory ] ERROR: Failed to map the memory!\nError code: %d\n",
        int32_t(result));
    return result;
  }
  if (!(memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
    pData = static_cast<uint8_t *>(pData) + inverseDeltaOffset;
    VkMappedMemoryRange mappedMemoryRange = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = handle,
        .offset = offset,
        .size = size};
    if (VkResult result = vkInvalidateMappedMemoryRanges(
            graphic::Singleton().Device(), 1, &mappedMemoryRange)) {
      printf("[ deviceMemory ] ERROR: Failed to flush the "
             "memory!\nError code: %d\n",
             int32_t(result));
      return result;
    }
  }
  return VK_SUCCESS;
}
// 取消映射host visible的内存区
VkResultThrowable
vulkanWrapper::deviceMemory::UnmapMemory(VkDeviceSize size,
                                         VkDeviceSize offset) const {
  if (!(memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
    AdjustNonCoherentMemoryRange(size, offset);
    VkMappedMemoryRange mappedMemoryRange = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = handle,
        .offset = offset,
        .size = size};
    if (VkResult result = vkFlushMappedMemoryRanges(
            graphic::Singleton().Device(), 1, &mappedMemoryRange)) {
      printf("[ deviceMemory ] ERROR: Failed to flush the "
             "memory!\nError code: %d\n",
             int32_t(result));
      return result;
    }
  }
  vkUnmapMemory(graphic::Singleton().Device(), handle);
  return VK_SUCCESS;
}
// BufferData(...)用于方便地更新设备内存区，适用于用memcpy(...)向内存区写入数据后立刻取消映射的情况
VkResultThrowable vulkanWrapper::deviceMemory::BufferData(
    const void *pData_src, VkDeviceSize size, VkDeviceSize offset) const {
  void *pData_dst;
  if (VkResult result = MapMemory(pData_dst, size, offset))
    return result;
  memcpy(pData_dst, pData_src, size_t(size));
  return UnmapMemory(size, offset);
}
// RetrieveData(...)用于方便地从设备内存区取回数据，适用于用memcpy(...)从内存区取得数据后立刻取消映射的情况
VkResultThrowable
vulkanWrapper::deviceMemory::RetrieveData(void *pData_dst, VkDeviceSize size,
                                          VkDeviceSize offset) const {
  void *pData_src;
  if (VkResult result = MapMemory(pData_src, size, offset))
    return result;
  memcpy(pData_dst, pData_src, size_t(size));
  return UnmapMemory(size, offset);
}
// Non-const Function
VkResultThrowable
vulkanWrapper::deviceMemory::Allocate(VkMemoryAllocateInfo &allocateInfo) {
  if (allocateInfo.memoryTypeIndex >=
      graphic::Singleton().PhysicalDeviceMemoryProperties().memoryTypeCount) {
    printf("[ deviceMemory ] ERROR: Invalid memory type index!\n");
    return VK_RESULT_MAX_ENUM; // 没有合适的错误代码，别用VK_ERROR_UNKNOWN
  }
  allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  if (VkResult result = vkAllocateMemory(graphic::Singleton().Device(),
                                         &allocateInfo, nullptr, &handle)) {
    printf(
        "[ deviceMemory ] ERROR: Failed to allocate memory!\nError code: %d\n",
        int32_t(result));
    return result;
  }
  // 记录实际分配的内存大小
  allocationSize = allocateInfo.allocationSize;
  // 取得内存属性
  memoryProperties = graphic::Singleton()
                         .PhysicalDeviceMemoryProperties()
                         .memoryTypes[allocateInfo.memoryTypeIndex]
                         .propertyFlags;
  return VK_SUCCESS;
};
