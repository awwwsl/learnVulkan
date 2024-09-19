#pragma once

#include "../Vulkan/formatInfo.hpp"

#include "../Models/graphic.hpp"

#include <math.h>
#include <memory>
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

  // Method for handling const uint8_t* address
  [[nodiscard]] static std::unique_ptr<uint8_t[]>
  LoadFile_MemoryAddress(const uint8_t *address, size_t fileSize,
                         VkExtent2D &extent, formatInfo requiredFormatInfo);
  // Method for handling const char* address
  [[nodiscard]]
  static std::unique_ptr<uint8_t[]>
  LoadFile_FileSystem(const char *address, VkExtent2D &extent,
                      formatInfo requiredFormatInfo);

  static inline uint32_t CalculateMipLevelCount(VkExtent2D extent) {
    return uint32_t(
               std::floor(std::log2(std::max(extent.width, extent.height)))) +
           1;
  }
  inline static VkSamplerCreateInfo DefaultSamplerCreateInfo() {
    return {.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .mipLodBias = 0.f,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = graphic::Singleton()
                                 .PhysicalDeviceProperties()
                                 .limits.maxSamplerAnisotropy,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.f,
            .maxLod = VK_LOD_CLAMP_NONE,
            .borderColor = {},
            .unnormalizedCoordinates = VK_FALSE};
  }
};
