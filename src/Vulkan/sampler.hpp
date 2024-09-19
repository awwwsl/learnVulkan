#pragma once

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

namespace vulkanWrapper {

class sampler {
  VkSampler handle = VK_NULL_HANDLE;

public:
  sampler();
  sampler(VkSamplerCreateInfo &createInfo);
  sampler(sampler &&other) noexcept;
  ~sampler();
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  // Non-const Function
  VkResultThrowable Create(VkSamplerCreateInfo &createInfo);
};

} // namespace vulkanWrapper
