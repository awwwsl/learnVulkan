#pragma once

#include "bufferMemory.hpp"

#include <cstring>

namespace vulkanWrapper {

class deviceLocalBuffer {
protected:
  bufferMemory memory;

public:
  deviceLocalBuffer();
  deviceLocalBuffer(VkDeviceSize size,
                    VkBufferUsageFlags desiredUsages_Without_transfer_dst);
  // Getter
  operator VkBuffer() const { return memory.Buffer(); }
  const VkBuffer *Address() const { return memory.AddressOfBuffer(); }
  VkDeviceSize AllocationSize() const { return memory.AllocationSize(); }
  // Const Function
  // 适用于更新连续的数据块
  void TransferData(const void *pData_src, VkDeviceSize size,
                    VkDeviceSize offset = 0) const;
  // 适用于更新不连续的多块数据，stride是每组数据间的步长，这里offset当然是目标缓冲区中的offset
  void TransferData(const void *pData_src, uint32_t elementCount,
                    VkDeviceSize elementSize, VkDeviceSize stride_src,
                    VkDeviceSize stride_dst, VkDeviceSize offset = 0) const;

  void CmdUpdateBuffer(VkCommandBuffer commandBuffer, const void *pData_src,
                       VkDeviceSize size_Limited_to_65536,
                       VkDeviceSize offset = 0) const;

  // Non-const Function
  void Create(VkDeviceSize size,
              VkBufferUsageFlags desiredUsages_Without_transfer_dst);
  void Recreate(VkDeviceSize size,
                VkBufferUsageFlags desiredUsages_Without_transfer_dst);
};

} // namespace vulkanWrapper
