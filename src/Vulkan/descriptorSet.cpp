#include "descriptorSet.hpp"

#include "../Models/graphic.hpp"

vulkanWrapper::descriptorSet::descriptorSet() = default;
vulkanWrapper::descriptorSet::descriptorSet(descriptorSet &&other) noexcept {
  MoveHandle;
}
// Const Function
void vulkanWrapper::descriptorSet::Write(
    std::vector<VkDescriptorImageInfo> descriptorInfos,
    VkDescriptorType descriptorType, uint32_t dstBinding,
    uint32_t dstArrayElement) const {
  VkWriteDescriptorSet writeDescriptorSet = {
      .dstSet = handle,
      .dstBinding = dstBinding,
      .dstArrayElement = dstArrayElement,
      .descriptorCount = uint32_t(descriptorInfos.size()),
      .descriptorType = descriptorType,
      .pImageInfo = descriptorInfos.data()};
  Update(writeDescriptorSet);
}
void vulkanWrapper::descriptorSet::Write(
    std::vector<VkDescriptorBufferInfo> descriptorInfos,
    VkDescriptorType descriptorType, uint32_t dstBinding,
    uint32_t dstArrayElement) const {
  VkWriteDescriptorSet writeDescriptorSet = {
      .dstSet = handle,
      .dstBinding = dstBinding,
      .dstArrayElement = dstArrayElement,
      .descriptorCount = uint32_t(descriptorInfos.size()),
      .descriptorType = descriptorType,
      .pBufferInfo = descriptorInfos.data()};
  Update(writeDescriptorSet);
}
void vulkanWrapper::descriptorSet::Write(
    std::vector<VkBufferView> descriptorInfos, VkDescriptorType descriptorType,
    uint32_t dstBinding, uint32_t dstArrayElement) const {
  VkWriteDescriptorSet writeDescriptorSet = {
      .dstSet = handle,
      .dstBinding = dstBinding,
      .dstArrayElement = dstArrayElement,
      .descriptorCount = uint32_t(descriptorInfos.size()),
      .descriptorType = descriptorType,
      .pTexelBufferView = descriptorInfos.data(),
  };
  Update(writeDescriptorSet);
}
void vulkanWrapper::descriptorSet::Write(
    std::vector<vulkanWrapper::bufferView> descriptorInfos,
    VkDescriptorType descriptorType, uint32_t dstBinding,
    uint32_t dstArrayElement) const {
  // HACK: copy instead of convert
  std::vector<VkBufferView> bufferViews(descriptorInfos.size());
  for (size_t i = 0; i < descriptorInfos.size(); i++)
    bufferViews[i] = descriptorInfos[i];
  Write(bufferViews, descriptorType, dstBinding, dstArrayElement);
}
// Static Function
void vulkanWrapper::descriptorSet::Update(
    std::vector<VkWriteDescriptorSet> writes,
    std::vector<VkCopyDescriptorSet> copies) {
  for (auto &i : writes)
    i.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  for (auto &i : copies)
    i.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  vkUpdateDescriptorSets(graphic::Singleton().Device(), writes.size(),
                         writes.data(), copies.size(), copies.data());
}

void vulkanWrapper::descriptorSet::Update(
    VkWriteDescriptorSet write, std::vector<VkCopyDescriptorSet> copies) {
  // HACK: copy instead of convert
  Update(std::vector<VkWriteDescriptorSet>{write}, copies);
}
