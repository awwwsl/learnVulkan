#pragma once

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

namespace vulkanWrapper {

class fence {
  VkFence handle = VK_NULL_HANDLE;

public:
  // fence() = default;
  inline fence(VkFenceCreateInfo &createInfo) { Create(createInfo); }

  // 默认构造器创建未置位的栅栏
  inline fence(VkFenceCreateFlags flags = 0) { Create(flags); }
  fence(fence &&other) noexcept;
  ~fence();

  // Getter
  inline DefineHandleTypeOperator;
  inline DefineAddressFunction;

  // Const Function
  VkResultThrowable Wait() const;
  VkResultThrowable Reset() const;
  VkResultThrowable WaitAndReset() const;
  VkResultThrowable Status() const;

  // Non-const Function
  VkResultThrowable Create(VkFenceCreateInfo &createInfo);
  inline VkResultThrowable Create(VkFenceCreateFlags flags = 0) {
    VkFenceCreateInfo createInfo = {.flags = flags};
    return Create(createInfo);
  }
};

} // namespace vulkanWrapper
