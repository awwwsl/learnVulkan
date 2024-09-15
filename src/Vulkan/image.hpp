#pragma once

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

#include <vulkan/vulkan.h>

namespace vulkanWrapper {

class image {
  VkImage handle = VK_NULL_HANDLE;

public:
  image();
  image(VkImageCreateInfo &createInfo);
  image(image &&other) noexcept;
  ~image();
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  // Const Function
  VkMemoryAllocateInfo
  MemoryAllocateInfo(VkMemoryPropertyFlags desiredMemoryProperties) const;
  VkResultThrowable BindMemory(VkDeviceMemory deviceMemory,
                               VkDeviceSize memoryOffset = 0) const;
  // Non-const Function
  VkResultThrowable Create(VkImageCreateInfo &createInfo);
};

} // namespace vulkanWrapper
