#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

namespace vulkanWrapper {

class buffer {
  VkBuffer handle = VK_NULL_HANDLE;

public:
  buffer();
  buffer(VkBufferCreateInfo &createInfo);
  buffer(buffer &&other) noexcept;
  ~buffer();
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  // Const Function
  VkMemoryAllocateInfo
  MemoryAllocateInfo(VkMemoryPropertyFlags desiredMemoryProperties) const;
  VkResultThrowable BindMemory(VkDeviceMemory deviceMemory,
                               VkDeviceSize memoryOffset = 0) const;
  // Non-const Function
  VkResultThrowable Create(VkBufferCreateInfo &createInfo);
};

} // namespace vulkanWrapper
