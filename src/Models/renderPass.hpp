#pragma once

#define GLFW_INCLUDE_VULKAN

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

#include <vector>
#include <vulkan/vulkan.h>

namespace vulkanWrapper {

class renderPass {
  VkRenderPass handle = VK_NULL_HANDLE;

public:
  renderPass();
  inline renderPass(VkRenderPassCreateInfo &createInfo) { Create(createInfo); }
  inline renderPass(renderPass &&other) noexcept;
  ~renderPass();
  // Getter
  DefineHandleTypeOperator;
  DefineAddressFunction;
  // Const Function
  void CmdBegin(
      VkCommandBuffer commandBuffer, VkRenderPassBeginInfo &beginInfo,
      VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const;
  void CmdBegin(
      VkCommandBuffer commandBuffer, VkFramebuffer framebuffer,
      VkRect2D renderArea, std::vector<VkClearValue> clearValues = {},
      VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const;
  void
  CmdNext(VkCommandBuffer commandBuffer,
          VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const;
  void CmdEnd(VkCommandBuffer commandBuffer) const;
  // Non-const Function
  VkResultThrowable Create(VkRenderPassCreateInfo &createInfo);
};

} // namespace vulkanWrapper
