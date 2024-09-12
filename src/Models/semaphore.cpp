#include "semaphore.hpp"
#include <stdio.h>

VkResultThrowable semaphore::Create(VkSemaphoreCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkResult result = vkCreateSemaphore(graphicsBase::Singleton().Device(),
                                      &createInfo, nullptr, &handle);
  if (result)
    printf("[ semaphore ] ERROR: Failed to create a "
           "semaphore!\nError code: %d\n",
           int32_t(result));
  return result;
};
