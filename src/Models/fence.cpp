#include "fence.hpp"
#include "graphic.hpp"

#include <stdio.h>
#include <vulkan/vulkan_core.h>

VkResultThrowable vulkanWrapper::fence::Wait() const {
  VkResult result = vkWaitForFences(graphicsBase::Singleton().Device(), 1,
                                    &handle, false, UINT64_MAX);
  if (result)
    printf("[ fence ] ERROR: Failed to wait for the fence!\nError code: %d\n",
           int32_t(result));
  return result;
}
VkResultThrowable vulkanWrapper::fence::Reset() const {
  VkResult result =
      vkResetFences(graphicsBase::Singleton().Device(), 1, &handle);
  if (result)
    printf("[ fence ] ERROR: Failed to reset the fence!\nError code: %d\n",
           int32_t(result));
  return result;
}
// 因为“等待后立刻重置”的情形经常出现，定义此函数
VkResultThrowable vulkanWrapper::fence::WaitAndReset() const {
  VkResult result = Wait();
  result || (result = Reset());
  return result;
}
VkResultThrowable vulkanWrapper::fence::Status() const {
  VkResult result =
      vkGetFenceStatus(graphicsBase::Singleton().Device(), handle);
  if (result <
      0) // vkGetFenceStatus(...)成功时有两种结果，所以不能仅仅判断result是否非0
    printf("[ fence ] ERROR: Failed to get the status of "
           "the fence!\nError code: %d\n",
           int32_t(result));
  return result;
}
// Non-const Function
VkResultThrowable vulkanWrapper::fence::Create(VkFenceCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  VkResult result = vkCreateFence(graphicsBase::Singleton().Device(),
                                  &createInfo, nullptr, &handle);
  if (result)
    printf("[ fence ] ERROR: Failed to create a fence!\nError code: %d\n",
           int32_t(result));
  return result;
};

vulkanWrapper::fence::fence(fence &&other) noexcept { MoveHandle; }
vulkanWrapper::fence::~fence() { DestroyHandleBy(vkDestroyFence, "fence"); }
