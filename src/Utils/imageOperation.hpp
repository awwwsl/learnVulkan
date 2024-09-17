#include <vulkan/vulkan.h>
// Texture
struct imageOperation {
  struct imageMemoryBarrierParameterPack {
    const bool isNeeded = false;
    const VkPipelineStageFlags stage = 0;
    const VkAccessFlags access = 0;
    const VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    constexpr imageMemoryBarrierParameterPack() = default;
    constexpr imageMemoryBarrierParameterPack(VkPipelineStageFlags stage,
                                              VkAccessFlags access,
                                              VkImageLayout layout)
        : stage(stage), access(access), layout(layout), isNeeded(true) {}
  };
  static void CmdCopyBufferToImage(VkCommandBuffer commandBuffer,
                                   VkBuffer buffer, VkImage image,
                                   const VkBufferImageCopy &region,
                                   imageMemoryBarrierParameterPack imb_from,
                                   imageMemoryBarrierParameterPack imb_to);
  static void CmdBlitImage(VkCommandBuffer commandBuffer, VkImage image_src,
                           VkImage image_dst, const VkImageBlit &region,
                           imageMemoryBarrierParameterPack imb_dst_from,
                           imageMemoryBarrierParameterPack imb_dst_to,
                           VkFilter filter = VK_FILTER_LINEAR);
  static void CmdGenerateMipmap2d(VkCommandBuffer commandBuffer, VkImage image,
                                  VkExtent2D imageExtent,
                                  uint32_t mipLevelCount, uint32_t layerCount,
                                  imageMemoryBarrierParameterPack imb_to,
                                  VkFilter minFilter = VK_FILTER_LINEAR);
};
