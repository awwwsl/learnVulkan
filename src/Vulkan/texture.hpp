#pragma once

#include "formatInfo.hpp"
#include "imageMemory.hpp"
#include "imageView.hpp"

#include "../Models/graphic.hpp"

#include "../Utils/imageOperation.hpp"

#include <math.h>
#include <memory>
#include <vulkan/vulkan_core.h>

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
  ~texture();

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
  // Static Function
  /*CheckArguments(...) should only be called in tests*/
  static bool CheckArguments(VkImageType imageType, VkExtent3D extent,
                             uint32_t arrayLayerCount, VkFormat format_initial,
                             VkFormat format_final, bool generateMipmap);
};

} // namespace vulkanWrapper
