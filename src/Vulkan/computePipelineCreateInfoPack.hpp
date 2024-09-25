#pragma once

#include <vector>
#include <vulkan/vulkan.h>

struct computePipelineCreateInfoPack {
  VkComputePipelineCreateInfo createInfo = {
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};

  computePipelineCreateInfoPack();
  computePipelineCreateInfoPack(
      const computePipelineCreateInfoPack &other) noexcept;

  // Getter
  operator VkComputePipelineCreateInfo &() { return createInfo; }

  // Non-const Function
  void UpdateAllArrays();

private:
  void SetCreateInfos();
  void UpdateAllArrayAddresses();
};

inline computePipelineCreateInfoPack::computePipelineCreateInfoPack() {
  SetCreateInfos();
}

inline computePipelineCreateInfoPack::computePipelineCreateInfoPack(
    const computePipelineCreateInfoPack &other) noexcept {
  createInfo = other.createInfo;
  SetCreateInfos();
}

inline void computePipelineCreateInfoPack::SetCreateInfos() {}

inline void computePipelineCreateInfoPack::UpdateAllArrays() {
  UpdateAllArrayAddresses();
}

inline void computePipelineCreateInfoPack::UpdateAllArrayAddresses() {}
