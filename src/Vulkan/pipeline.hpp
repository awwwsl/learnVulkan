#pragma once

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

namespace vulkanWrapper {

class pipeline {

  VkPipeline handle = VK_NULL_HANDLE;

public:
  pipeline() = default;
  pipeline(VkGraphicsPipelineCreateInfo &createInfo) { Create(createInfo); }
  pipeline(VkComputePipelineCreateInfo &createInfo) { Create(createInfo); }
  pipeline(pipeline &&other) noexcept;
  ~pipeline();
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  // Non-const Function
  VkResultThrowable Create(VkGraphicsPipelineCreateInfo &createInfo);
  VkResultThrowable Create(VkComputePipelineCreateInfo &createInfo);
};

} // namespace vulkanWrapper
