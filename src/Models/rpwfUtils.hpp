#pragma once

#include "../Vulkan/depthStencilAttachment.hpp"
#include "../Vulkan/framebuffer.hpp"
#include "../Vulkan/renderPass.hpp"

namespace rpwfUtils {

struct renderPassWithFramebuffer {
  vulkanWrapper::renderPass renderPass;
  vulkanWrapper::framebuffer framebuffer;
};
struct renderPassWithFramebuffers {
  vulkanWrapper::renderPass renderPass;
  std::vector<vulkanWrapper::framebuffer> framebuffers;
};

const renderPassWithFramebuffers &CreateRpwf_Screen();

const renderPassWithFramebuffer &CreateRpwf_Canvas(VkExtent2D canvasSize);
void CmdClearCanvas(VkCommandBuffer commandBuffer,
                    VkClearColorValue clearColor);

const renderPassWithFramebuffers &CreateRpwf_ScreenWithDS(
    VkFormat depthStencilFormat,
    std::vector<vulkanWrapper::depthStencilAttachment> &dsas_screenWithDS);

const renderPassWithFramebuffers &CreateRpwf_DeferredToScreen(
    VkFormat depthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT);

} // namespace rpwfUtils
