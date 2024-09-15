#pragma once

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

namespace vulkanWrapper {

class pipelineLayout {
  VkPipelineLayout handle = VK_NULL_HANDLE;

public:
  pipelineLayout() = default;
  pipelineLayout(VkPipelineLayoutCreateInfo &createInfo) { Create(createInfo); }
  pipelineLayout(pipelineLayout &&other) noexcept;
  ~pipelineLayout();
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  // Non-const Function
  VkResultThrowable Create(VkPipelineLayoutCreateInfo &createInfo);
};

} // namespace vulkanWrapper
