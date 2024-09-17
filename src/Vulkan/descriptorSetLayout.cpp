#include "descriptorSetLayout.hpp"
#include "../Models/graphic.hpp"

vulkanWrapper::descriptorSetLayout::descriptorSetLayout() = default;
vulkanWrapper::descriptorSetLayout::descriptorSetLayout(
    VkDescriptorSetLayoutCreateInfo &createInfo) {
  Create(createInfo);
}
vulkanWrapper::descriptorSetLayout::descriptorSetLayout(
    descriptorSetLayout &&other) noexcept {
  MoveHandle;
}
vulkanWrapper::descriptorSetLayout::~descriptorSetLayout() {
  DestroyHandleBy(vkDestroyDescriptorSetLayout, "descriptorSetLayout");
}
// Non-const Function
VkResultThrowable vulkanWrapper::descriptorSetLayout::Create(
    VkDescriptorSetLayoutCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  VkResult result = vkCreateDescriptorSetLayout(graphic::Singleton().Device(),
                                                &createInfo, nullptr, &handle);
  if (result)
    printf("[ descriptorSetLayout ] ERROR: Failed to create "
           "a descriptor set layout!\nError code: %d\n",
           int32_t(result));
  return result;
}
