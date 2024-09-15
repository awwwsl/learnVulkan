#include "image.hpp"

#include "graphic.hpp"

vulkanWrapper::image::image() = default;
vulkanWrapper::image::image(VkImageCreateInfo &createInfo) {
  Create(createInfo);
}
vulkanWrapper::image::image(image &&other) noexcept { MoveHandle; }
vulkanWrapper::image::~image() { DestroyHandleBy(vkDestroyImage, "image"); }
// Const Function
VkMemoryAllocateInfo vulkanWrapper::image::MemoryAllocateInfo(
    VkMemoryPropertyFlags desiredMemoryProperties) const {
  VkMemoryAllocateInfo memoryAllocateInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  VkMemoryRequirements memoryRequirements;
  vkGetImageMemoryRequirements(graphic::Singleton().Device(), handle,
                               &memoryRequirements);
  memoryAllocateInfo.allocationSize = memoryRequirements.size;
  auto GetMemoryTypeIndex = [](uint32_t memoryTypeBits,
                               VkMemoryPropertyFlags desiredMemoryProperties) {
    auto &physicalDeviceMemoryProperties =
        graphic::Singleton().PhysicalDeviceMemoryProperties();
    for (size_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
      if (memoryTypeBits & 1 << i &&
          (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags &
           desiredMemoryProperties) == desiredMemoryProperties)
        return i;
    return UINT64_MAX;
  };
  memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(
      memoryRequirements.memoryTypeBits, desiredMemoryProperties);
  if (memoryAllocateInfo.memoryTypeIndex == UINT32_MAX &&
      desiredMemoryProperties & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
    memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(
        memoryRequirements.memoryTypeBits,
        desiredMemoryProperties & ~VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
  // 不在此检查是否成功取得内存类型索引，因为会把memoryAllocateInfo返回出去，交由外部检查
  // if (memoryAllocateInfo.memoryTypeIndex == -1)
  //     outStream << std::format("[ image ] ERROR\nFailed to find any memory
  //     type satisfies all desired memory properties!\n");
  return memoryAllocateInfo;
}
VkResultThrowable
vulkanWrapper::image::BindMemory(VkDeviceMemory deviceMemory,
                                 VkDeviceSize memoryOffset) const {
  VkResult result = vkBindImageMemory(graphic::Singleton().Device(), handle,
                                      deviceMemory, memoryOffset);
  if (result)
    printf("[ image ] ERROR: Failed to attach the memory!\nError code: %d\n",
           int32_t(result));
  return result;
}
// Non-const Function
VkResultThrowable vulkanWrapper::image::Create(VkImageCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  VkResult result = vkCreateImage(graphic::Singleton().Device(), &createInfo,
                                  nullptr, &handle);
  if (result)
    printf("[ image ] ERROR: Failed to create an image!\nError code: %d\n",
           int32_t(result));
  return result;
}
