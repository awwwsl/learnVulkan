#pragma once

#include "imageMemory.hpp"
#include "imageView.hpp"

namespace vulkanWrapper {

class attachment {
protected:
  imageView view;
  imageMemory memory;
  //--------------------
  attachment();

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
  // 该函数返回写入描述符时需要的信息
  VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler) const;
};

} // namespace vulkanWrapper
