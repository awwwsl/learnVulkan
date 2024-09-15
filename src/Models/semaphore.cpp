#define GLFW_INCLUDE_VULKAN

#include "semaphore.hpp"
#include "graphic.hpp"

#include <stdio.h>

VkResultThrowable
vulkanWrapper::semaphore::Create(VkSemaphoreCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkResult result = vkCreateSemaphore(graphicsBase::Singleton().Device(),
                                      &createInfo, nullptr, &handle);
  if (result)
    printf("[ semaphore ] ERROR: Failed to create a "
           "semaphore!\nError code: %d\n",
           int32_t(result));
  return result;
};

vulkanWrapper::semaphore::semaphore(semaphore &&other) noexcept { MoveHandle; }
vulkanWrapper::semaphore::~semaphore() {
  DestroyHandleBy(vkDestroySemaphore, "semaphore");
}
