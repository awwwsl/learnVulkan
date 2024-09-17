#include "attachment.hpp"
#include <vulkan/vulkan_core.h>

vulkanWrapper::attachment::attachment() = default;

VkDescriptorImageInfo
vulkanWrapper::attachment::DescriptorImageInfo(VkSampler sampler) const {
  return {sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
}
