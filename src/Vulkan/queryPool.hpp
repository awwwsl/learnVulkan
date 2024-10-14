#include <vulkan/vulkan_core.h>

#include "Utils/Macros.hpp"
#include "Utils/VkResultThrowable.hpp"

namespace vulkanWrapper {
class queryPool {
  VkQueryPool handle = VK_NULL_HANDLE;

  DefineHandleTypeOperator;
  DefineAddressFunction;

public:
  queryPool();
  ~queryPool();

  queryPool(queryPool &&other) noexcept;

  VkResultThrowable Create(VkQueryPoolCreateInfo &createInfo);
};
}; // namespace vulkanWrapper
