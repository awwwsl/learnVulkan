#include "imageView.hpp"

#include "../Models/graphic.hpp"

vulkanWrapper::imageView::imageView() = default;
vulkanWrapper::imageView::imageView(VkImageViewCreateInfo &createInfo) {
  Create(createInfo);
}
vulkanWrapper::imageView::imageView(
    VkImage image, VkImageViewType viewType, VkFormat format,
    const VkImageSubresourceRange &subresourceRange,
    VkImageViewCreateFlags flags) {
  Create(image, viewType, format, subresourceRange, flags);
}
vulkanWrapper::imageView::imageView(imageView &&other) noexcept { MoveHandle; }
vulkanWrapper::imageView::~imageView() {
  DestroyHandleBy(vkDestroyImageView, "imageView");
}
// Non-const Function
VkResultThrowable
vulkanWrapper::imageView::Create(VkImageViewCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  VkResult result = vkCreateImageView(graphic::Singleton().Device(),
                                      &createInfo, nullptr, &handle);
  if (result)
    printf("[ imageView ] ERROR: Failed to create an image "
           "view!\nError code: %d\n",
           int32_t(result));
  return result;
}
VkResultThrowable vulkanWrapper::imageView::Create(
    VkImage image, VkImageViewType viewType, VkFormat format,
    const VkImageSubresourceRange &subresourceRange,
    VkImageViewCreateFlags flags) {
  VkImageViewCreateInfo createInfo = {.flags = flags,
                                      .image = image,
                                      .viewType = viewType,
                                      .format = format,
                                      .subresourceRange = subresourceRange};
  return Create(createInfo);
}
