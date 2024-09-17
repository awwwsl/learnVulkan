#pragma once

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

#include "descriptorSet.hpp"
#include "descriptorSetLayout.hpp"

#include <vector>

namespace vulkanWrapper {

class descriptorPool {
  VkDescriptorPool handle = VK_NULL_HANDLE;

public:
  descriptorPool();
  descriptorPool(VkDescriptorPoolCreateInfo &createInfo);
  descriptorPool(uint32_t maxSetCount,
                 std::vector<VkDescriptorPoolSize> poolSizes,
                 VkDescriptorPoolCreateFlags flags = 0);
  descriptorPool(descriptorPool &&other) noexcept;
  ~descriptorPool();
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  // Const Function
  VkResultThrowable
  AllocateSets(std::vector<VkDescriptorSet> &sets,
               std::vector<VkDescriptorSetLayout> &setLayouts) const;
  VkResultThrowable
  AllocateSets(std::vector<VkDescriptorSet> &sets,
               std::vector<descriptorSetLayout> &setLayouts) const;
  VkResultThrowable
  AllocateSets(std::vector<descriptorSet> &sets,
               std::vector<VkDescriptorSetLayout> &setLayouts) const;
  VkResultThrowable
  AllocateSets(std::vector<descriptorSet> &sets,
               std::vector<descriptorSetLayout> &setLayouts) const;
  VkResultThrowable FreeSets(std::vector<VkDescriptorSet> &sets) const;
  VkResultThrowable FreeSets(std::vector<descriptorSet> &sets) const;
  // Non-const Function
  VkResultThrowable Create(VkDescriptorPoolCreateInfo &createInfo);
  VkResultThrowable Create(uint32_t maxSetCount,
                           std::vector<VkDescriptorPoolSize> poolSizes,
                           VkDescriptorPoolCreateFlags flags = 0);
};

} // namespace vulkanWrapper
