#include "rpwfUtils.hpp"

#include "graphic.hpp"

#include "../Vulkan/colorAttachment.hpp"
#include "../Vulkan/depthStencilAttachment.hpp"

vulkanWrapper::colorAttachment ca_canvas;
const rpwfUtils::renderPassWithFramebuffers &rpwfUtils::CreateRpwf_Screen() {
  static renderPassWithFramebuffers rpwf;

  VkAttachmentDescription attachmentDescription = {
      .format = graphic::Singleton().SwapchainCreateInfo().imageFormat,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
  VkAttachmentReference attachmentReference = {
      0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  VkSubpassDescription subpassDescription = {
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentReference};
  VkSubpassDependency subpassDependency = {
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT};
  VkRenderPassCreateInfo renderPassCreateInfo = {
      .attachmentCount = 1,
      .pAttachments = &attachmentDescription,
      .subpassCount = 1,
      .pSubpasses = &subpassDescription,
      .dependencyCount = 1,
      .pDependencies = &subpassDependency};
  rpwf.renderPass.Create(renderPassCreateInfo);
  auto CreateFramebuffers = [] {
    const VkExtent2D &windowSize =
        graphic::Singleton().SwapchainCreateInfo().imageExtent;
    rpwf.framebuffers.resize(graphic::Singleton().SwapchainImageCount());
    VkFramebufferCreateInfo framebufferCreateInfo = {
        .renderPass = rpwf.renderPass,
        .attachmentCount = 1,
        .width = windowSize.width,
        .height = windowSize.height,
        .layers = 1};
    for (size_t i = 0; i < graphic::Singleton().SwapchainImageCount(); i++) {
      VkImageView attachment = graphic::Singleton().SwapchainImageView(i);
      framebufferCreateInfo.pAttachments = &attachment;
      rpwf.framebuffers[i].Create(framebufferCreateInfo);
    }
  };
  auto DestroyFramebuffers = [] { rpwf.framebuffers.clear(); };
  CreateFramebuffers();

  ExecuteOnce(rpwf);
  graphic::Singleton().AddCreateSwapchainCallback(CreateFramebuffers);
  graphic::Singleton().AddDestroySwapchainCallback(DestroyFramebuffers);
  return rpwf;
}
const rpwfUtils::renderPassWithFramebuffer &
rpwfUtils::CreateRpwf_Canvas(VkExtent2D canvasSize) {
  static renderPassWithFramebuffer rpwf;
  // When this render pass begins, the image keeps its contents.
  VkAttachmentDescription attachmentDescription = {
      .format = graphic::Singleton().SwapchainCreateInfo().imageFormat,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  VkAttachmentReference attachmentReference = {
      0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  VkSubpassDescription subpassDescription = {
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentReference};
  VkSubpassDependency subpassDependencies[2] = {
      {.srcSubpass = VK_SUBPASS_EXTERNAL,
       .dstSubpass = 0,
       // You may use VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT if synchronization is
       // done by fence.
       .srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
       .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
       .srcAccessMask = 0,
       .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
       .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT},
      {.srcSubpass = 0,
       .dstSubpass = VK_SUBPASS_EXTERNAL,
       .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
       .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
       .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
       .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
       .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT}};
  VkRenderPassCreateInfo renderPassCreateInfo = {
      .attachmentCount = 1,
      .pAttachments = &attachmentDescription,
      .subpassCount = 1,
      .pSubpasses = &subpassDescription,
      .dependencyCount = 2,
      .pDependencies = subpassDependencies,
  };
  rpwf.renderPass.Create(renderPassCreateInfo);
  ca_canvas.Create(graphic::Singleton().SwapchainCreateInfo().imageFormat,
                   canvasSize, 1, VK_SAMPLE_COUNT_1_BIT,
                   VK_IMAGE_USAGE_SAMPLED_BIT |
                       VK_IMAGE_USAGE_TRANSFER_DST_BIT);
  VkFramebufferCreateInfo framebufferCreateInfo = {
      .renderPass = rpwf.renderPass,
      .attachmentCount = 1,
      .pAttachments = ca_canvas.AddressOfImageView(),
      .width = canvasSize.width,
      .height = canvasSize.height,
      .layers = 1};
  rpwf.framebuffer.Create(framebufferCreateInfo);
  return rpwf;
}

void rpwfUtils::CmdClearCanvas(VkCommandBuffer commandBuffer,
                               VkClearColorValue clearColor) {
  // Call this function before rpwf.renderPass begins.
  VkImageSubresourceRange imageSubresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0,
                                                   1, 0, 1};
  VkImageMemoryBarrier imageMemoryBarrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = ca_canvas.Image(),
      .subresourceRange = imageSubresourceRange};
  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &imageMemoryBarrier);
  vkCmdClearColorImage(commandBuffer, ca_canvas.Image(),
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1,
                       &imageSubresourceRange);
  imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  imageMemoryBarrier.dstAccessMask = 0;
  imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                       nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

const rpwfUtils::renderPassWithFramebuffers &rpwfUtils::CreateRpwf_ScreenWithDS(
    VkFormat depthStencilFormat,
    std::vector<vulkanWrapper::depthStencilAttachment> &dsas_screenWithDS) {
  static renderPassWithFramebuffers rpwf;
  static VkFormat _depthStencilFormat = depthStencilFormat;

  VkAttachmentDescription attachmentDescriptions[2] = {
      {// Color attachment
       .format = graphic::Singleton().SwapchainCreateInfo().imageFormat,
       .samples = VK_SAMPLE_COUNT_1_BIT,
       .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
       .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
       .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
       .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
       .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
      {// Depth stencil attachment
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
       .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
  VkAttachmentReference attachmentReferences[2] = {
      {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
      // Unless the separateDepthStencilLayouts feature is enabled, even if
      // depthStencilFormat doesn't support stencil, layout must be
      // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL.
      {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
  VkSubpassDescription subpassDescription = {
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = attachmentReferences,
      .pDepthStencilAttachment = attachmentReferences + 1};
  // At EARLY_FRAGMENT_TESTS stage, the ds image'll be cleared (if performs)
  // then readed, ds tests are performed for each fragment. At
  // LATE_FRAGMENT_TESTS stage, ds tests are performed for each sample.
  VkSubpassDependency subpassDependency = {
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .srcAccessMask = 0,
      .dstAccessMask =
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, // Because of
                                                        // VK_ATTACHMENT_LOAD_OP_CLEAR
      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT};
  VkRenderPassCreateInfo renderPassCreateInfo = {
      .attachmentCount = 2,
      .pAttachments = attachmentDescriptions,
      .subpassCount = 1,
      .pSubpasses = &subpassDescription,
      .dependencyCount = 1,
      .pDependencies = &subpassDependency};
  rpwf.renderPass.Create(renderPassCreateInfo);
  auto CreateFramebuffers = [&dsas_screenWithDS] {
    const VkExtent2D &windowSize =
        graphic::Singleton().SwapchainCreateInfo().imageExtent;
    dsas_screenWithDS.resize(graphic::Singleton().SwapchainImageCount());
    rpwf.framebuffers.resize(graphic::Singleton().SwapchainImageCount());
    for (auto &i : dsas_screenWithDS)
      i.Create(_depthStencilFormat, windowSize, 1, VK_SAMPLE_COUNT_1_BIT,
               VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
    VkFramebufferCreateInfo framebufferCreateInfo = {
        .renderPass = rpwf.renderPass,
        .attachmentCount = 2,
        .width = windowSize.width,
        .height = windowSize.height,
        .layers = 1};
    for (size_t i = 0; i < graphic::Singleton().SwapchainImageCount(); i++) {
      VkImageView attachments[2] = {graphic::Singleton().SwapchainImageView(i),
                                    dsas_screenWithDS[i].ImageView()};
      framebufferCreateInfo.pAttachments = attachments;
      rpwf.framebuffers[i].Create(framebufferCreateInfo);
    }
  };
  auto DestroyFramebuffers = [&dsas_screenWithDS] {
    dsas_screenWithDS.clear();
    rpwf.framebuffers.clear();
  };
  CreateFramebuffers();

  ExecuteOnce(rpwf);
  graphic::Singleton().AddCreateSwapchainCallback(CreateFramebuffers);
  graphic::Singleton().AddDestroySwapchainCallback(DestroyFramebuffers);
  return rpwf;
}

// vulkanWrapper::colorAttachment ca_deferredToScreen_normalZ;
// vulkanWrapper::colorAttachment ca_deferredToScreen_albedoSpecular;
// vulkanWrapper::depthStencilAttachment dsa_deferredToScreen;
// const rpwfUtils::renderPassWithFramebuffers &
// rpwfUtils::CreateRpwf_DeferredToScreen(VkFormat depthStencilFormat) {
//   static renderPassWithFramebuffers rpwf;
//   static VkFormat _depthStencilFormat = depthStencilFormat;
//
//   VkAttachmentDescription attachmentDescriptions[4] = {
//       {// Swapchain attachment
//        .format = graphic::Singleton().SwapchainCreateInfo().imageFormat,
//        .samples = VK_SAMPLE_COUNT_1_BIT,
//        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
//        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
//        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
//       {// Deferred normal & z attachment
//        .format =
//            VK_FORMAT_R16G16B16A16_SFLOAT, // Or VK_FORMAT_R32G32B32A32_SFLOAT
//        .samples = VK_SAMPLE_COUNT_1_BIT,
//        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
//        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//        .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
//       {// Deffered albedo & specular attachment
//        .format = VK_FORMAT_R8G8B8A8_UNORM, // The only difference from above
//        .samples = VK_SAMPLE_COUNT_1_BIT,
//        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
//        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//        .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
//       {// Depth stencil attachment
//        .format = _depthStencilFormat,
//        .samples = VK_SAMPLE_COUNT_1_BIT,
//        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
//        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//        .stencilLoadOp = _depthStencilFormat >= VK_FORMAT_S8_UINT
//                             ? VK_ATTACHMENT_LOAD_OP_CLEAR
//                             : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
//   VkAttachmentReference attachmentReferences_subpass0[3] = {
//       {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
//       {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
//       {3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
//   VkAttachmentReference attachmentReferences_subpass1[3] = {
//       {1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
//       {2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
//       {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
//   VkSubpassDescription subpassDescriptions[2] = {
//       {.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
//        .colorAttachmentCount = 2,
//        .pColorAttachments = attachmentReferences_subpass0,
//        .pDepthStencilAttachment = attachmentReferences_subpass0 + 2},
//       {.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
//        .inputAttachmentCount = 2,
//        .pInputAttachments = attachmentReferences_subpass1,
//        .colorAttachmentCount = 1,
//        .pColorAttachments = attachmentReferences_subpass1 + 2}};
//   VkSubpassDependency subpassDependencies[2] = {
//       {.srcSubpass = VK_SUBPASS_EXTERNAL,
//        .dstSubpass = 0,
//        .srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//        .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
//        .srcAccessMask = 0,
//        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
//        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT},
//       {.srcSubpass = 0,
//        .dstSubpass = 1,
//        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//        .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
//        .dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
//        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT}};
//   VkRenderPassCreateInfo renderPassCreateInfo = {
//       .attachmentCount = 4,
//       .pAttachments = attachmentDescriptions,
//       .subpassCount = 2,
//       .pSubpasses = subpassDescriptions,
//       .dependencyCount = 2,
//       .pDependencies = subpassDependencies};
//   rpwf.renderPass.Create(renderPassCreateInfo);
//   auto CreateFramebuffers = [] {
//     const VkExtent2D &windowSize =
//         graphic::Singleton().SwapchainCreateInfo().imageExtent;
//     rpwf.framebuffers.resize(graphic::Singleton().SwapchainImageCount());
//     ca_deferredToScreen_normalZ.Create(
//         VK_FORMAT_R16G16B16A16_SFLOAT, windowSize, 1, VK_SAMPLE_COUNT_1_BIT,
//         VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
//             VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
//     ca_deferredToScreen_albedoSpecular.Create(
//         VK_FORMAT_R8G8B8A8_UNORM, windowSize, 1, VK_SAMPLE_COUNT_1_BIT,
//         VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
//             VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
//     dsa_deferredToScreen.Create(_depthStencilFormat, windowSize, 1,
//                                 VK_SAMPLE_COUNT_1_BIT,
//                                 VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
//     VkImageView attachments[4] = {
//         VK_NULL_HANDLE, ca_deferredToScreen_normalZ.ImageView(),
//         ca_deferredToScreen_albedoSpecular.ImageView(),
//         dsa_deferredToScreen.ImageView()};
//     VkFramebufferCreateInfo framebufferCreateInfo = {
//         .renderPass = rpwf.renderPass,
//         .attachmentCount = 4,
//         .pAttachments = attachments,
//         .width = windowSize.width,
//         .height = windowSize.height,
//         .layers = 1};
//     for (size_t i = 0; i < graphic::Singleton().SwapchainImageCount(); i++)
//       attachments[0] = graphic::Singleton().SwapchainImageView(i),
//       rpwf.framebuffers[i].Create(framebufferCreateInfo);
//   };
//   auto DestroyFramebuffers = [] {
//     ca_deferredToScreen_normalZ.~colorAttachment();
//     ca_deferredToScreen_albedoSpecular.~colorAttachment();
//     dsa_deferredToScreen.~depthStencilAttachment();
//     rpwf.framebuffers.clear();
//   };
//   CreateFramebuffers();
//
//   ExecuteOnce(rpwf);
//   graphic::Singleton().AddCreateSwapchainCallback(CreateFramebuffers);
//   graphic::Singleton().AddDestroySwapchainCallback(DestroyFramebuffers);
//   return rpwf;
// }
