#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

#include <vulkan/vulkan.h>

namespace vulkanWrapper {

class bufferView {
  VkBufferView handle = VK_NULL_HANDLE;

public:
  bufferView();
  bufferView(VkBufferViewCreateInfo &createInfo);
  bufferView(VkBuffer buffer, VkFormat format, VkDeviceSize offset = 0,
             VkDeviceSize range = 0 /*VkBufferViewCreateFlags flags*/);
  bufferView(bufferView &&other) noexcept;
  ~bufferView();
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  // Non-const Function
  VkResultThrowable Create(VkBufferViewCreateInfo &createInfo);
  VkResultThrowable
  Create(VkBuffer buffer, VkFormat format, VkDeviceSize offset = 0,
         VkDeviceSize range = 0 /*VkBufferViewCreateFlags flags*/);
};

} // namespace vulkanWrapper
