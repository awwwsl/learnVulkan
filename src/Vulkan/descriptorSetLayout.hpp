#pragma once

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

namespace vulkanWrapper {

class descriptorSetLayout {
  VkDescriptorSetLayout handle = VK_NULL_HANDLE;

public:
  descriptorSetLayout();
  descriptorSetLayout(VkDescriptorSetLayoutCreateInfo &createInfo);
  descriptorSetLayout(descriptorSetLayout &&other) noexcept;
  ~descriptorSetLayout();
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  // Non-const Function
  VkResultThrowable Create(VkDescriptorSetLayoutCreateInfo &createInfo);
};

} // namespace vulkanWrapper
