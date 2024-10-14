#include "queryPool.hpp"
#include "Macros.hpp"
#include "Models/graphic.hpp"
#include "VkResultThrowable.hpp"

#include <vulkan/vulkan_core.h>

vulkanWrapper::queryPool::queryPool() = default;
vulkanWrapper::queryPool::queryPool(queryPool &&other) noexcept { MoveHandle; }
vulkanWrapper::queryPool::~queryPool() {
  DestroyHandleBy(vkDestroyQueryPool, "queryPool");
}

VkResultThrowable
vulkanWrapper::queryPool::Create(VkQueryPoolCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
  VkResult result = vkCreateQueryPool(graphic::Singleton().Device(),
                                      &createInfo, nullptr, &handle);

  if (result)
    printf("[ queryPool ] ERROR: Failed to create a "
           "query pool!\nError code: %d\n",
           int32_t(result));
#ifndef NDEBUG
  printf("[ queryPool ] DEBUG: QueryPool created with handle %p\n", handle);
#endif
  return result;
}
