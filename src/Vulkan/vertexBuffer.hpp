#pragma once

#include "deviceLocalBuffer.hpp"

namespace vulkanWrapper {

class vertexBuffer : public deviceLocalBuffer {
public:
  vertexBuffer() = default;
  vertexBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0)
      : deviceLocalBuffer(size,
                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | otherUsages) {}
  // Non-const Function
  void Create(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
    deviceLocalBuffer::Create(size,
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | otherUsages);
  }
  void Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0) {
    deviceLocalBuffer::Recreate(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                          otherUsages);
  }
};

} // namespace vulkanWrapper
