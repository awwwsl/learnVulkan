#include "graphic.hpp"

#include "graphicPlus.hpp"
#include "graphicPlusImpl.hpp"

#include "../Vulkan/fence.hpp"

graphicPlusImpl::graphicPlusImpl() {
  auto Initialize = [this] {
    if (graphic::Singleton().QueueFamilyIndex_Graphics() !=
        VK_QUEUE_FAMILY_IGNORED)
      Singleton().commandPool_graphics.Create(
          graphic::Singleton().QueueFamilyIndex_Graphics(),
          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
          Singleton().commandPool_graphics.AllocateBuffers(
              Singleton().commandBuffer_transfer);
    if (graphic::Singleton().QueueFamilyIndex_Compute() !=
        VK_QUEUE_FAMILY_IGNORED)
      Singleton().commandPool_compute.Create(
          graphic::Singleton().QueueFamilyIndex_Compute(),
          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    if (graphic::Singleton().QueueFamilyIndex_Presentation() !=
            VK_QUEUE_FAMILY_IGNORED &&
        graphic::Singleton().QueueFamilyIndex_Presentation() !=
            graphic::Singleton().QueueFamilyIndex_Graphics() &&
        graphic::Singleton().SwapchainCreateInfo().imageSharingMode ==
            VK_SHARING_MODE_EXCLUSIVE)
      Singleton().commandPool_presentation.Create(
          graphic::Singleton().QueueFamilyIndex_Presentation(),
          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
          Singleton().commandPool_presentation.AllocateBuffers(
              Singleton().commandBuffer_presentation);
    // 新增------------------------------------
    for (size_t i = 0; i < std::size(Singleton().formatProperties); i++)
      vkGetPhysicalDeviceFormatProperties(graphic::Singleton().PhysicalDevice(),
                                          VkFormat(i),
                                          &Singleton().formatProperties[i]);
    //----------------------------------------
  };
  auto CleanUp = [this] {
    Singleton().commandPool_graphics.~commandPool();
    Singleton().commandPool_presentation.~commandPool();
    Singleton().commandPool_compute.~commandPool();
  };
  this->CleanUpLambda = CleanUp;
  graphic::Singleton().AddCreateDeviceCallback(Initialize);
  graphic::Singleton().AddDestroyDeviceCallback(CleanUp);
}

graphicPlusImpl::~graphicPlusImpl() = default;

// Const Function
// 简化命令提交
VkResultThrowable graphicPlusImpl::ExecuteCommandBuffer_Graphics(
    VkCommandBuffer commandBuffer) const {
  vulkanWrapper::fence fence;
  VkSubmitInfo submitInfo = {.commandBufferCount = 1,
                             .pCommandBuffers = &commandBuffer};
  VkResult result =
      graphic::Singleton().SubmitCommandBuffer_Graphics(submitInfo, fence);
  if (!result)
    fence.Wait();
  return result;
}
VkResultThrowable graphicPlusImpl::ExecuteCommandBuffer_Compute(
    VkCommandBuffer commandBuffer) const {
  vulkanWrapper::fence fence;
  VkSubmitInfo submitInfo = {.commandBufferCount = 1,
                             .pCommandBuffers = &commandBuffer};
  VkResult result =
      graphic::Singleton().SubmitCommandBuffer_Compute(submitInfo, fence);
  if (!result)
    fence.Wait();
  return result;
}
// 该函数专用于向呈现队列提交用于接收交换链图像的队列族所有权的命令缓冲区
VkResultThrowable graphicPlusImpl::AcquireImageOwnership_Presentation(
    VkSemaphore semaphore_renderingIsOver,
    VkSemaphore semaphore_ownershipIsTransfered, VkFence fence) const {
  if (VkResult result = commandBuffer_presentation.Begin(
          VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT))
    return result;
  graphic::Singleton().CmdTransferImageOwnership(commandBuffer_presentation);
  if (VkResult result = commandBuffer_presentation.End())
    return result;
  return graphic::Singleton().SubmitCommandBuffer_Presentation(
      commandBuffer_presentation, semaphore_renderingIsOver,
      semaphore_ownershipIsTransfered, fence);
}
