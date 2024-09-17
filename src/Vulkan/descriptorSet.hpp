#pragma once

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

#include "bufferView.hpp"

#include <vector>

namespace vulkanWrapper {

class descriptorSet {
  friend class descriptorPool;
  VkDescriptorSet handle = VK_NULL_HANDLE;

public:
  descriptorSet();
  descriptorSet(descriptorSet &&other) noexcept;
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  // Const Function
  void Write(std::vector<VkDescriptorImageInfo> descriptorInfos,
             VkDescriptorType descriptorType, uint32_t dstBinding = 0,
             uint32_t dstArrayElement = 0) const;
  void Write(std::vector<VkDescriptorBufferInfo> descriptorInfos,
             VkDescriptorType descriptorType, uint32_t dstBinding = 0,
             uint32_t dstArrayElement = 0) const;
  void Write(std::vector<VkBufferView> descriptorInfos,
             VkDescriptorType descriptorType, uint32_t dstBinding = 0,
             uint32_t dstArrayElement = 0) const;
  void Write(std::vector<vulkanWrapper::bufferView> descriptorInfos,
             VkDescriptorType descriptorType, uint32_t dstBinding = 0,
             uint32_t dstArrayElement = 0) const;
  // Static Function
  static void Update(std::vector<VkWriteDescriptorSet> writes,
                     std::vector<VkCopyDescriptorSet> copies = {});

  static void Update(VkWriteDescriptorSet write,
                     std::vector<VkCopyDescriptorSet> copies = {});
};

} // namespace vulkanWrapper
