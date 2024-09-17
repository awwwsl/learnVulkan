#include "buffer.hpp"

#include "../Models/graphic.hpp"

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

vulkanWrapper::buffer::buffer() = default;
vulkanWrapper::buffer::buffer(VkBufferCreateInfo &createInfo) {
  Create(createInfo);
}
vulkanWrapper::buffer::buffer(buffer &&other) noexcept { MoveHandle; }
vulkanWrapper::buffer::~buffer() { DestroyHandleBy(vkDestroyBuffer, "buffer"); }

// Const Function
VkMemoryAllocateInfo vulkanWrapper::buffer::MemoryAllocateInfo(
    VkMemoryPropertyFlags desiredMemoryProperties) const {
  VkMemoryAllocateInfo memoryAllocateInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(graphic::Singleton().Device(), handle,
                                &memoryRequirements);
  memoryAllocateInfo.allocationSize = memoryRequirements.size;
  memoryAllocateInfo.memoryTypeIndex = UINT32_MAX;
  auto &physicalDeviceMemoryProperties =
      graphic::Singleton().PhysicalDeviceMemoryProperties();
  for (size_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
    if (memoryRequirements.memoryTypeBits & 1 << i &&
        (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags &
         desiredMemoryProperties) == desiredMemoryProperties) {
      memoryAllocateInfo.memoryTypeIndex = i;
      break;
    }
  // 不在此检查是否成功取得内存类型索引，因为会把memoryAllocateInfo返回出去，交由外部检查
  // if (memoryAllocateInfo.memoryTypeIndex == UINT32_MAX)
  //     outStream << std::format("[ buffer ] ERROR\nFailed to find any memory
  //     type satisfies all desired memory properties!\n");
  return memoryAllocateInfo;
}
VkResultThrowable
vulkanWrapper::buffer::BindMemory(VkDeviceMemory deviceMemory,
                                  VkDeviceSize memoryOffset) const {
  VkResult result = vkBindBufferMemory(graphic::Singleton().Device(), handle,
                                       deviceMemory, memoryOffset);
  if (result)
    printf("[ buffer ] ERROR: Failed to attach the memory!\nError code: %d\n",
           int32_t(result));
  return result;
}
// Non-const Function
VkResultThrowable
vulkanWrapper::buffer::Create(VkBufferCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  VkResult result = vkCreateBuffer(graphic::Singleton().Device(), &createInfo,
                                   nullptr, &handle);
  if (result)
    printf("[ buffer ] ERROR: Failed to create a buffer!\nError code: %d\n",
           int32_t(result));
#ifndef NDEBUG
  printf("[ buffer ] Buffer created with handle %p\n", handle);
#endif
  return result;
};
