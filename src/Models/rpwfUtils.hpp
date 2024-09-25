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
const renderPassWithFramebuffers &CreateRenderPassWithFramebuffers(
    VkFormat depthStencilFormat,
    std::vector<vulkanWrapper::depthStencilAttachment>
        &depthStencilAttachments);
} // namespace rpwfUtils
