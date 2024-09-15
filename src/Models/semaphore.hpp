#pragma once

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"
#include <vulkan/vulkan.h>

namespace vulkanWrapper {

class semaphore {
  VkSemaphore handle = VK_NULL_HANDLE;

public:
  // semaphore() = default;
  inline semaphore(VkSemaphoreCreateInfo &createInfo) { Create(createInfo); }

  // 默认构造器创建未置位的信号量
  inline semaphore(/*VkSemaphoreCreateFlags flags*/) { Create(); }
  semaphore(semaphore &&other) noexcept;
  ~semaphore();

  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;

  // Non-const Function
  VkResultThrowable Create(VkSemaphoreCreateInfo &createInfo);
  inline VkResultThrowable Create(/*VkSemaphoreCreateFlags flags*/) {
    VkSemaphoreCreateInfo createInfo = {};
    return Create(createInfo);
  }
};

} // namespace vulkanWrapper
