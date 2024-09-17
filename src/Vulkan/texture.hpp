#pragma once

#include "formatInfo.hpp"
#include "imageMemory.hpp"
#include "imageView.hpp"

#include "../Models/graphic.hpp"

#include "../Utils/imageOperation.hpp"

#include <math.h>
#include <memory>

namespace vulkanWrapper {

class texture {
protected:
  imageView view;
  imageMemory memory;
  //--------------------
  texture();
  void CreateImageMemory(VkImageType imageType, VkFormat format,
                         VkExtent3D extent, uint32_t mipLevelCount,
                         uint32_t arrayLayerCount,
                         VkImageCreateFlags flags = 0);
  void CreateImageView(VkImageViewType viewType, VkFormat format,
                       uint32_t mipLevelCount, uint32_t arrayLayerCount,
                       VkImageViewCreateFlags flags = 0);

public:
  // Getter
  inline VkImageView ImageView() const { return view; }
  inline VkImage Image() const { return memory.Image(); }
  inline const VkImageView *AddressOfImageView() const {
    return view.Address();
  }
  inline const VkImage *AddressOfImage() const {
    return memory.AddressOfImage();
  }
  // Const Function
  VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler) const {
    return {sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  }
  // Static Function
  /*CheckArguments(...) should only be called in tests*/
  static bool CheckArguments(VkImageType imageType, VkExtent3D extent,
                             uint32_t arrayLayerCount, VkFormat format_initial,
                             VkFormat format_final, bool generateMipmap);

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
  static void CopyBlitAndGenerateMipmap2d(
      VkBuffer buffer_copyFrom, VkImage image_copyTo, VkImage image_blitTo,
      VkExtent2D imageExtent, uint32_t mipLevelCount = 1,
      uint32_t layerCount = 1, VkFilter minFilter = VK_FILTER_LINEAR);
  static void BlitAndGenerateMipmap2d(VkImage image_preinitialized,
                                      VkImage image_final,
                                      VkExtent2D imageExtent,
                                      uint32_t mipLevelCount = 1,
                                      uint32_t layerCount = 1,
                                      VkFilter minFilter = VK_FILTER_LINEAR);
  inline static VkSamplerCreateInfo SamplerCreateInfo() {
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

} // namespace vulkanWrapper
