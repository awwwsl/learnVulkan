#pragma once

#include "formatInfo.hpp"
#include "imageMemory.hpp"
#include "imageView.hpp"

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vulkanWrapper {
class dynamicTexture {
protected:
  std::vector<imageView> views;
  imageMemory memory;
  uint32_t currentView = 0;
  //--------------------
  dynamicTexture();
  ~dynamicTexture();
  dynamicTexture(const dynamicTexture &) = delete;
  void CreateImageMemory(VkImageType imageType, VkFormat format,
                         VkExtent3D extent, uint32_t mipLevelCount,
                         uint32_t arrayLayerCount,
                         VkImageCreateFlags flags = 0);
  void CreateImageViews(VkImageViewType viewType, VkFormat format,
                        uint32_t mipLevelCount, uint32_t arrayLayerCount,
                        VkImageViewCreateFlags flags = 0);

public:
  // Getter
  inline std::vector<VkImageView> ImageViews() const {
    std::vector<VkImageView> viewsTransport(views.size());
    for (int i = 0; i < views.size(); i++) {
      viewsTransport[i] = views[i];
    }
    return viewsTransport;
  }
  inline VkImageView ImageView(uint32_t index) const { return views[index]; }
  inline VkImageView CurrentView() const { return views[currentView]; }
  inline uint32_t UpdateView() {
    currentView++;
    currentView %= views.size();
    return currentView;
  }
  inline int viewCount() { return views.size(); }
  inline VkImage Image() const { return memory.Image(); }
};
} // namespace vulkanWrapper
