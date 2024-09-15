#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

#include <stdlib.h>
#include <vulkan/vulkan.h>

namespace vulkanWrapper {

class deviceMemory {
  VkDeviceMemory handle = VK_NULL_HANDLE;
  VkDeviceSize allocationSize = 0;            // 实际分配的内存大小
  VkMemoryPropertyFlags memoryProperties = 0; // 内存属性
  //--------------------
  // 该函数用于在映射内存区时，调整非host coherent的内存区域的范围
  VkDeviceSize AdjustNonCoherentMemoryRange(VkDeviceSize &size,
                                            VkDeviceSize &offset) const;

protected:
  // 用于bufferMemory或imageMemory，定义于此以节省8个字节
  class {
    friend class bufferMemory;
    friend class imageMemory;
    bool value = false;
    operator bool() const { return value; }
    auto &operator=(bool value) {
      this->value = value;
      return *this;
    }
  } areBound;

public:
  deviceMemory();
  deviceMemory(VkMemoryAllocateInfo &allocateInfo);
  deviceMemory(deviceMemory &&other) noexcept;
  ~deviceMemory();
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  inline VkDeviceSize AllocationSize() const { return allocationSize; }
  inline VkMemoryPropertyFlags MemoryProperties() const {
    return memoryProperties;
  }

  // Const Function
  // 映射host visible的内存区
  VkResultThrowable MapMemory(void *&pData, VkDeviceSize size,
                              VkDeviceSize offset = 0) const;
  // 取消映射host visible的内存区
  VkResultThrowable UnmapMemory(VkDeviceSize size,
                                VkDeviceSize offset = 0) const;
  // BufferData(...)用于方便地更新设备内存区，适用于用memcpy(...)向内存区写入数据后立刻取消映射的情况
  VkResultThrowable BufferData(const void *pData_src, VkDeviceSize size,
                               VkDeviceSize offset = 0) const;
  // RetrieveData(...)用于方便地从设备内存区取回数据，适用于用memcpy(...)从内存区取得数据后立刻取消映射的情况
  VkResultThrowable RetrieveData(void *pData_dst, VkDeviceSize size,
                                 VkDeviceSize offset = 0) const;
  // Non-const Function
  VkResultThrowable Allocate(VkMemoryAllocateInfo &allocateInfo);
};

} // namespace vulkanWrapper
