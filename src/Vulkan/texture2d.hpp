#pragma once

#include "texture.hpp"

namespace vulkanWrapper {

class texture2d : public texture {
protected:
  VkExtent2D extent = {};
  //--------------------
  void Create_Internal(VkFormat format_initial, VkFormat format_final,
                       bool generateMipmap, VkFilter blitFilter);

public:
  texture2d();
  inline texture2d(const char *filepath, VkFormat format_initial,
                   VkFormat format_final, bool generateMipmap = true,
                   VkFilter blitFilter = VK_FILTER_LINEAR) {
    Create(filepath, format_initial, format_final, generateMipmap, blitFilter);
  }
  inline texture2d(const uint8_t *pImageData, VkExtent2D extent,
                   VkFormat format_initial, VkFormat format_final,
                   bool generateMipmap = true,
                   VkFilter blitFilter = VK_FILTER_LINEAR) {
    Create(pImageData, extent, format_initial, format_final, generateMipmap,
           blitFilter);
  }
  // Getter
  inline VkExtent2D Extent() const { return extent; }
  inline uint32_t Width() const { return extent.width; }
  inline uint32_t Height() const { return extent.height; }
  // Non-const Function
  // 直接从硬盘读取文件
  void Create(const char *filepath, VkFormat format_initial,
              VkFormat format_final, bool generateMipmap = true,
              VkFilter blitFilter = VK_FILTER_LINEAR);
  // 从内存读取文件数据
  void Create(const uint8_t *pImageData, VkExtent2D extent,
              VkFormat format_initial, VkFormat format_final,
              bool generateMipmap = true,
              VkFilter blitFilter = VK_FILTER_LINEAR);
};

} // namespace vulkanWrapper
