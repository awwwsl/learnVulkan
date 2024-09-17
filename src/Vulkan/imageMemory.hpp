#pragma once

#include "deviceMemory.hpp"
#include "image.hpp"

#include "../Utils/VkResultThrowable.hpp"

namespace vulkanWrapper {

class imageMemory : image, deviceMemory {
public:
  imageMemory();
  imageMemory(VkImageCreateInfo &createInfo,
              VkMemoryPropertyFlags desiredMemoryProperties);
  imageMemory(imageMemory &&other) noexcept;
  ~imageMemory();
  // Getter
  inline VkImage Image() const { return static_cast<const image &>(*this); }
  inline const VkImage *AddressOfImage() const { return image::Address(); }
  inline VkDeviceMemory Memory() const {
    return static_cast<const deviceMemory &>(*this);
  }
  inline const VkDeviceMemory *AddressOfMemory() const {
    return deviceMemory::Address();
  }
  inline bool AreBound() const { return areBound; }

  using deviceMemory::AllocationSize;
  using deviceMemory::MemoryProperties;
  // Non-const Function
  // 以下三个函数仅用于Create(...)可能执行失败的情况
  VkResultThrowable CreateImage(VkImageCreateInfo &createInfo);
  VkResultThrowable
  AllocateMemory(VkMemoryPropertyFlags desiredMemoryProperties);
  VkResultThrowable BindMemory();
  // 分配设备内存、创建图像、绑定
  VkResultThrowable Create(VkImageCreateInfo &createInfo,
                           VkMemoryPropertyFlags desiredMemoryProperties);
};

} // namespace vulkanWrapper
