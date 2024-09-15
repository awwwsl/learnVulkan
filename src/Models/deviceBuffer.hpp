#include "buffer.hpp"
#include "deviceMemory.hpp"

namespace vulkanWrapper {

class bufferMemory : buffer, deviceMemory {
public:
  bufferMemory();
  bufferMemory(VkBufferCreateInfo &createInfo,
               VkMemoryPropertyFlags desiredMemoryProperties);
  bufferMemory(bufferMemory &&other) noexcept;
  ~bufferMemory();

  // Getter
  // 不定义到VkBuffer和VkDeviceMemory的转换函数，因为32位下这俩类型都是uint64_t的别名，会造成冲突（虽然，谁他妈还用32位PC！）
  VkBuffer Buffer() const { return static_cast<const buffer &>(*this); }
  const VkBuffer *AddressOfBuffer() const { return buffer::Address(); }
  VkDeviceMemory Memory() const {
    return static_cast<const deviceMemory &>(*this);
  }
  const VkDeviceMemory *AddressOfMemory() const {
    return deviceMemory::Address();
  }
  // 若areBond为true，则成功分配了设备内存、创建了缓冲区，且成功绑定在一起
  bool AreBound() const { return areBound; }
  using deviceMemory::AllocationSize;
  using deviceMemory::MemoryProperties;
  // Const Function
  using deviceMemory::BufferData;
  using deviceMemory::MapMemory;
  using deviceMemory::RetrieveData;
  using deviceMemory::UnmapMemory;
  // Non-const Function
  // 以下三个函数仅用于Create(...)可能执行失败的情况
  VkResultThrowable CreateBuffer(VkBufferCreateInfo &createInfo);
  VkResultThrowable
  AllocateMemory(VkMemoryPropertyFlags desiredMemoryProperties);
  VkResultThrowable BindMemory();

  // 分配设备内存、创建缓冲、绑定
  VkResultThrowable Create(VkBufferCreateInfo &createInfo,
                           VkMemoryPropertyFlags desiredMemoryProperties);
};

} // namespace vulkanWrapper
