#include "renderPass.hpp"
#include "graphic.hpp"
#include <stdio.h>

void vulkanWrapper::renderPass::CmdBegin(
    VkCommandBuffer commandBuffer, VkRenderPassBeginInfo &beginInfo,
    VkSubpassContents subpassContents) const {
  beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  beginInfo.renderPass = handle;
  vkCmdBeginRenderPass(commandBuffer, &beginInfo, subpassContents);
}
void vulkanWrapper::renderPass::CmdBegin(
    VkCommandBuffer commandBuffer, VkFramebuffer framebuffer,
    VkRect2D renderArea, std::vector<VkClearValue> clearValues,
    VkSubpassContents subpassContents) const {
  VkRenderPassBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = handle,
      .framebuffer = framebuffer,
      .renderArea = renderArea,
      .clearValueCount = uint32_t(clearValues.size()),
      .pClearValues = clearValues.data()};
  vkCmdBeginRenderPass(commandBuffer, &beginInfo, subpassContents);
}
void vulkanWrapper::renderPass::CmdNext(
    VkCommandBuffer commandBuffer, VkSubpassContents subpassContents) const {
  vkCmdNextSubpass(commandBuffer, subpassContents);
}
void vulkanWrapper::renderPass::CmdEnd(VkCommandBuffer commandBuffer) const {
  vkCmdEndRenderPass(commandBuffer);
}
// Non-const Function
VkResultThrowable
vulkanWrapper::renderPass::Create(VkRenderPassCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  VkResult result = vkCreateRenderPass(graphicsBase::Singleton().Device(),
                                       &createInfo, nullptr, &handle);
  if (result)
    printf("[ renderPass ] ERROR: Failed to create a "
           "render pass!\nError code: %d\n",
           int32_t(result));
  return result;
}

vulkanWrapper::renderPass::renderPass() {}

vulkanWrapper::renderPass::renderPass(renderPass &&other) noexcept {
  MoveHandle;
}
vulkanWrapper::renderPass::~renderPass() {
  DestroyHandleBy(vkDestroyRenderPass, "renderPass");
}
