#include "descriptorPool.hpp"

#include "../Models/graphic.hpp"
#include <cstring>

vulkanWrapper::descriptorPool::descriptorPool() = default;
vulkanWrapper::descriptorPool::descriptorPool(
    VkDescriptorPoolCreateInfo &createInfo) {
  Create(createInfo);
}
vulkanWrapper::descriptorPool::descriptorPool(
    uint32_t maxSetCount, std::vector<VkDescriptorPoolSize> poolSizes,
    VkDescriptorPoolCreateFlags flags) {
  Create(maxSetCount, poolSizes, flags);
}
vulkanWrapper::descriptorPool::descriptorPool(descriptorPool &&other) noexcept {
  MoveHandle;
}
vulkanWrapper::descriptorPool::~descriptorPool() {
  DestroyHandleBy(vkDestroyDescriptorPool, "descriptorPool");
}
// Const Function
VkResultThrowable vulkanWrapper::descriptorPool::AllocateSets(
    std::vector<VkDescriptorSet> &sets,
    std::vector<VkDescriptorSetLayout> &setLayouts) const {
  if (sets.size() != setLayouts.size()) {
    if (sets.size() < setLayouts.size()) {
      printf(
          "[ descriptorPool ] ERROR: For each descriptor set, must provide a "
          "corresponding layout!\n");
      return VK_RESULT_MAX_ENUM; // 没有合适的错误代码，别用VK_ERROR_UNKNOWN
    } else {
      printf("[ descriptorPool ] WARNING: Provided layouts "
             "are more than sets!\n");
    }
  }
  VkDescriptorSetAllocateInfo allocateInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = handle,
      .descriptorSetCount = uint32_t(sets.size()),
      .pSetLayouts = setLayouts.data()};
  VkResult result = vkAllocateDescriptorSets(graphic::Singleton().Device(),
                                             &allocateInfo, sets.data());
  if (result)
    printf("[ descriptorPool ] ERROR: Failed to allocate "
           "descriptor sets!\nError code: %d\n",
           int32_t(result));

  return result;
}
VkResultThrowable vulkanWrapper::descriptorPool::AllocateSets(
    std::vector<VkDescriptorSet> &sets,
    std::vector<descriptorSetLayout> &setLayouts) const {
  std::vector<VkDescriptorSetLayout> vkSetLayouts(setLayouts.size());
  for (size_t i = 0; i < setLayouts.size(); i++)
    vkSetLayouts[i] = setLayouts[i];
  return AllocateSets(sets, vkSetLayouts);
}
VkResultThrowable vulkanWrapper::descriptorPool::AllocateSets(
    std::vector<descriptorSet> &sets,
    std::vector<VkDescriptorSetLayout> &setLayouts) const {
  std::vector<VkDescriptorSet> vkSets(sets.size());
  for (size_t i = 0; i < sets.size(); i++)
    vkSets[i] = sets[i];
  return AllocateSets(vkSets, setLayouts);
}
VkResultThrowable vulkanWrapper::descriptorPool::AllocateSets(
    std::vector<descriptorSet> &sets,
    std::vector<descriptorSetLayout> &setLayouts) const {
  std::vector<VkDescriptorSet> vkSets(sets.size());
  std::vector<VkDescriptorSetLayout> vkSetLayouts(setLayouts.size());
  for (size_t i = 0; i < sets.size(); i++)
    vkSets[i] = sets[i];
  for (size_t i = 0; i < setLayouts.size(); i++)
    vkSetLayouts[i] = setLayouts[i];
  return AllocateSets(vkSets, vkSetLayouts);
}
VkResultThrowable vulkanWrapper::descriptorPool::FreeSets(
    std::vector<VkDescriptorSet> &sets) const {
  VkResult result = vkFreeDescriptorSets(graphic::Singleton().Device(), handle,
                                         sets.size(), sets.data());
  memset(sets.data(), 0, sets.size() * sizeof(VkDescriptorSet));
  return result; // Though vkFreeDescriptorSets(...) can only return
                 // VK_SUCCESS
}
VkResultThrowable vulkanWrapper::descriptorPool::FreeSets(
    std::vector<descriptorSet> &sets) const {
  std::vector<VkDescriptorSet> vkSets(sets.size());
  for (size_t i = 0; i < sets.size(); i++)
    vkSets[i] = sets[i];
  return FreeSets(vkSets);
}
// Non-const Function
VkResultThrowable
vulkanWrapper::descriptorPool::Create(VkDescriptorPoolCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  VkResult result = vkCreateDescriptorPool(graphic::Singleton().Device(),
                                           &createInfo, nullptr, &handle);
  if (result)
    printf("[ descriptorPool ] ERROR: Failed to create a "
           "descriptor pool!\nError code: %d\n",
           int32_t(result));
  return result;
}
VkResultThrowable vulkanWrapper::descriptorPool::Create(
    uint32_t maxSetCount, std::vector<VkDescriptorPoolSize> poolSizes,
    VkDescriptorPoolCreateFlags flags) {
  VkDescriptorPoolCreateInfo createInfo = {.flags = flags,
                                           .maxSets = maxSetCount,
                                           .poolSizeCount =
                                               uint32_t(poolSizes.size()),
                                           .pPoolSizes = poolSizes.data()};
  return Create(createInfo);
}
