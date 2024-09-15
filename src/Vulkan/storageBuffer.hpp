#pragma once

#include "../Models/graphic.hpp"

#include "deviceLocalBuffer.hpp"

namespace vulkanWrapper {

class storageBuffer : public deviceLocalBuffer {
public:
  storageBuffer() = default;
  storageBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0)
      : deviceLocalBuffer(size,
                          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | otherUsages) {}
  // Non-const Function
  void Create(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
    deviceLocalBuffer::Create(size,
                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | otherUsages);
  }
  void Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
    deviceLocalBuffer::Recreate(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                          otherUsages);
  }
  // Static Function
  static VkDeviceSize CalculateAlignedSize(VkDeviceSize dataSize) {
    const VkDeviceSize &alignment = graphic::Singleton()
                                        .PhysicalDeviceProperties()
                                        .limits.minStorageBufferOffsetAlignment;
    return dataSize + alignment - 1 & ~(alignment - 1);
  }
};

} // namespace vulkanWrapper
