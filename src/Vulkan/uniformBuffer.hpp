#pragma once

#include "../Models/graphic.hpp"

#include "deviceLocalBuffer.hpp"

namespace vulkanWrapper {

class uniformBuffer : public deviceLocalBuffer {
public:
  uniformBuffer() = default;
  uniformBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0)
      : deviceLocalBuffer(size,
                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | otherUsages) {}
  // Non-const Function
  void Create(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
    deviceLocalBuffer::Create(size,
                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | otherUsages);
  }
  void Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
    deviceLocalBuffer::Recreate(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                          otherUsages);
  }
  // Static Function
  static VkDeviceSize CalculateAlignedSize(VkDeviceSize dataSize) {
    const VkDeviceSize &alignment = graphic::Singleton()
                                        .PhysicalDeviceProperties()
                                        .limits.minUniformBufferOffsetAlignment;
    return dataSize + alignment - 1 & ~(alignment - 1);
  }
};

} // namespace vulkanWrapper
