#include "../Models/graphic.hpp"
#include "../Models/graphicPlus.hpp"

#include "deviceLocalBuffer.hpp"
#include "stagingBuffer.hpp"

vulkanWrapper::deviceLocalBuffer::deviceLocalBuffer() = default;
vulkanWrapper::deviceLocalBuffer::deviceLocalBuffer(
    VkDeviceSize size, VkBufferUsageFlags desiredUsages_Without_transfer_dst) {
  Create(size, desiredUsages_Without_transfer_dst);
}

void vulkanWrapper::deviceLocalBuffer::TransferData(const void *pData_src,
                                                    VkDeviceSize size,
                                                    VkDeviceSize offset) const {
  if (memory.MemoryProperties() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
    memory.BufferData(pData_src, size, offset);
    return;
  }
  stagingBuffer::BufferData_CurrentThread(pData_src, size);
  auto &commandBuffer = graphic::Plus().CommandBuffer_Transfer();
  commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  VkBufferCopy region = {0, offset, size};
  vkCmdCopyBuffer(commandBuffer, stagingBuffer::Buffer_CurrentThread(),
                  memory.Buffer(), 1, &region);
  commandBuffer.End();
  graphic::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);
}
// 适用于更新不连续的多块数据，stride是每组数据间的步长，这里offset当然是目标缓冲区中的offset
void vulkanWrapper::deviceLocalBuffer::TransferData(const void *pData_src,
                                                    uint32_t elementCount,
                                                    VkDeviceSize elementSize,
                                                    VkDeviceSize stride_src,
                                                    VkDeviceSize stride_dst,
                                                    VkDeviceSize offset) const {
  if (memory.MemoryProperties() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
    void *pData_dst = nullptr;
    memory.MapMemory(pData_dst, stride_dst * elementCount, offset);
    for (size_t i = 0; i < elementCount; i++)
      memcpy(stride_dst * i + static_cast<uint8_t *>(pData_dst),
             stride_src * i + static_cast<const uint8_t *>(pData_src),
             size_t(elementSize));
    memory.UnmapMemory(elementCount * stride_dst, offset);
    return;
  }
  stagingBuffer::BufferData_CurrentThread(pData_src, stride_src * elementCount);
  const vulkanWrapper::commandBuffer &commandBuffer =
      graphic::Plus().CommandBuffer_Transfer();
  commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  std::unique_ptr<VkBufferCopy[]> regions =
      std::make_unique<VkBufferCopy[]>(elementCount);
  for (size_t i = 0; i < elementCount; i++)
    regions[i] = {stride_src * i, stride_dst * i + offset, elementSize};
  vkCmdCopyBuffer(commandBuffer, stagingBuffer::Buffer_CurrentThread(),
                  memory.Buffer(), elementCount, regions.get());
  commandBuffer.End();
  graphic::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);
}

void vulkanWrapper::deviceLocalBuffer::CmdUpdateBuffer(
    VkCommandBuffer commandBuffer, const void *pData_src,
    VkDeviceSize size_Limited_to_65536, VkDeviceSize offset) const {
  vkCmdUpdateBuffer(commandBuffer, memory.Buffer(), offset,
                    size_Limited_to_65536, pData_src);
}

// Non-const Function
void vulkanWrapper::deviceLocalBuffer::Create(
    VkDeviceSize size, VkBufferUsageFlags desiredUsages_Without_transfer_dst) {
  VkBufferCreateInfo bufferCreateInfo = {
      .size = size,
      .usage = desiredUsages_Without_transfer_dst |
               VK_BUFFER_USAGE_TRANSFER_DST_BIT};
  false || memory.CreateBuffer(bufferCreateInfo) ||
      memory.AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
          memory.AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ||
      memory.BindMemory();
}
void vulkanWrapper::deviceLocalBuffer::Recreate(
    VkDeviceSize size, VkBufferUsageFlags desiredUsages_Without_transfer_dst) {
  graphic::Singleton().WaitIdle();
  memory.~bufferMemory();
  Create(size, desiredUsages_Without_transfer_dst);
}
