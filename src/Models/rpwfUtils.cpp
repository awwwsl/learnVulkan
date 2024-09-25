#include "rpwfUtils.hpp"

#include "graphic.hpp"

#include "../Vulkan/colorAttachment.hpp"
#include "../Vulkan/depthStencilAttachment.hpp"
#include <vulkan/vulkan_core.h>

const rpwfUtils::renderPassWithFramebuffers &
rpwfUtils::CreateRenderPassWithFramebuffers(
    VkFormat depthStencilFormat,
    std::vector<vulkanWrapper::depthStencilAttachment>
        &depthstencilAttachments) {
  static renderPassWithFramebuffers rpwf;
  static VkFormat _depthStencilFormat = depthStencilFormat;

  VkAttachmentDescription attachmentDescriptions[] = {
      {
          // Color attachment
          .format = graphic::Singleton().SwapchainCreateInfo().imageFormat,
          .samples = VK_SAMPLE_COUNT_1_BIT,
          .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
          .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
          .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
          .finalLayout = VK_IMAGE_LAYOUT_GENERAL,
      },
      {
          // Depth stencil attachment
          .format = _depthStencilFormat,
          .samples = VK_SAMPLE_COUNT_1_BIT,
          .loadOp = _depthStencilFormat != VK_FORMAT_S8_UINT
                        ? VK_ATTACHMENT_LOAD_OP_CLEAR
                        : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
          .stencilLoadOp = _depthStencilFormat >= VK_FORMAT_S8_UINT
                               ? VK_ATTACHMENT_LOAD_OP_CLEAR
                               : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
          .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      },
  };
  VkAttachmentReference attachmentReferences_subpass1[] = {
      {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
      // Unless the separateDepthStencilLayouts feature is enabled, even if
      // depthStencilFormat doesn't support stencil, layout must be
      // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL.
      {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
  };
  VkAttachmentReference attachmentReferences_subpass2[] = {
      {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
  };
  VkSubpassDescription subpassDescriptions[] = {
      {
          .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
          .colorAttachmentCount = 1,
          .pColorAttachments = attachmentReferences_subpass1,
          .pDepthStencilAttachment = attachmentReferences_subpass1 + 1,
      },
      // {
      //     .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      //     .colorAttachmentCount = 1,
      //     .pColorAttachments = attachmentReferences_subpass2,
      //     .pDepthStencilAttachment = nullptr,
      // }};
  };
  // At EARLY_FRAGMENT_TESTS stage, the ds image'll be cleared (if performs)
  // then readed, ds tests are performed for each fragment. At
  // LATE_FRAGMENT_TESTS stage, ds tests are performed for each sample.
  VkSubpassDependency subpassDependencies[] = {
      {
          .srcSubpass = VK_SUBPASS_EXTERNAL,
          .dstSubpass = 0,
          .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
          .srcAccessMask = 0,
          .dstAccessMask =
              VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // Because of
                                                    // VK_ATTACHMENT_LOAD_OP_CLEAR
          .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
      },
      // {
      //     .srcSubpass = 0,
      //     .dstSubpass = 1,
      //     .srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
      //     .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      //     .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      //     .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
      //                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      //     .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
      // },
  };
  VkRenderPassCreateInfo renderPassCreateInfo = {
      .attachmentCount = 2,
      .pAttachments = attachmentDescriptions,
      .subpassCount = 1,
      .pSubpasses = subpassDescriptions,
      .dependencyCount = 1,
      .pDependencies = subpassDependencies,
  };
  rpwf.renderPass.Create(renderPassCreateInfo);
  auto CreateFramebuffers = [&depthstencilAttachments] {
    const VkExtent2D &windowSize =
        graphic::Singleton().SwapchainCreateInfo().imageExtent;
    depthstencilAttachments.resize(graphic::Singleton().SwapchainImageCount());
    rpwf.framebuffers.resize(graphic::Singleton().SwapchainImageCount());
    for (auto &i : depthstencilAttachments)
      i.Create(_depthStencilFormat, windowSize, 1, VK_SAMPLE_COUNT_1_BIT,
               VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
    VkFramebufferCreateInfo framebufferCreateInfo = {
        .renderPass = rpwf.renderPass,
        .attachmentCount = 2,
        .width = windowSize.width,
        .height = windowSize.height,
        .layers = 1,
    };
    for (size_t i = 0; i < graphic::Singleton().SwapchainImageCount(); i++) {
      VkImageView attachments[2] = {graphic::Singleton().SwapchainImageView(i),
                                    depthstencilAttachments[i].ImageView()};
      framebufferCreateInfo.pAttachments = attachments;
      rpwf.framebuffers[i].Create(framebufferCreateInfo);
    }
  };
  auto DestroyFramebuffers = [&depthstencilAttachments] {
    depthstencilAttachments.clear();
    rpwf.framebuffers.clear();
  };
  CreateFramebuffers();

  ExecuteOnce(rpwf);
  graphic::Singleton().AddCreateSwapchainCallback(CreateFramebuffers);
  graphic::Singleton().AddDestroySwapchainCallback(DestroyFramebuffers);
  return rpwf;
}
