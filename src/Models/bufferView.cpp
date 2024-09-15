#include "graphic.hpp"

#include "bufferView.hpp"

vulkanWrapper::bufferView::bufferView() = default;
vulkanWrapper::bufferView::bufferView(VkBufferViewCreateInfo &createInfo) {
  Create(createInfo);
}
vulkanWrapper::bufferView::bufferView(
    VkBuffer buffer, VkFormat format, VkDeviceSize offset,
    VkDeviceSize range /*VkBufferViewCreateFlags flags*/) {
  Create(buffer, format, offset, range);
}
vulkanWrapper::bufferView::bufferView(bufferView &&other) noexcept {
  MoveHandle;
}
vulkanWrapper::bufferView::~bufferView(){
    DestroyHandleBy(vkDestroyBufferView, "bufferView")} // Non-const Function
VkResultThrowable
    vulkanWrapper::bufferView::Create(VkBufferViewCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
  VkResult result = vkCreateBufferView(graphic::Singleton().Device(),
                                       &createInfo, nullptr, &handle);
  if (result)
    printf("[ bufferView ] ERROR: Failed to create a buffer "
           "view!\nError code: %d\n",
           int32_t(result));
  return result;
}
VkResultThrowable vulkanWrapper::bufferView::Create(
    VkBuffer buffer, VkFormat format, VkDeviceSize offset,
    VkDeviceSize range /*VkBufferViewCreateFlags flags*/) {
  VkBufferViewCreateInfo createInfo = {
      .buffer = buffer, .format = format, .offset = offset, .range = range};
  return Create(createInfo);
}
