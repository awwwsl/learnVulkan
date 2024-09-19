#include "graphicPlus.hpp"
#include "graphicPlusImpl.hpp"

graphicPlus &graphicPlus::Singleton() {
  static graphicPlus instance; // 单例实例
  return instance;
}

graphicPlus::graphicPlus() { impl = &graphicPlusImpl::Singleton(); }
graphicPlus::~graphicPlus() {}

// Getter
const VkFormatProperties &graphicPlus::FormatProperties(VkFormat format) const {
  return impl->FormatProperties(format);
}
const vulkanWrapper::commandPool &graphicPlus::CommandPool_Graphics() const {
  return impl->commandPool_graphics;
}
const vulkanWrapper::commandPool &graphicPlus::CommandPool_Compute() const {
  return impl->commandPool_compute;
}
const vulkanWrapper::commandBuffer &
graphicPlus::CommandBuffer_Transfer() const {
  return impl->commandBuffer_transfer;
}

// Const Fuctions
VkResultThrowable graphicPlus::ExecuteCommandBuffer_Graphics(
    VkCommandBuffer commandBuffer) const {
  return impl->ExecuteCommandBuffer_Graphics(commandBuffer);
}
VkResultThrowable
graphicPlus::ExecuteCommandBuffer_Compute(VkCommandBuffer commandBuffer) const {
  return impl->ExecuteCommandBuffer_Compute(commandBuffer);
}
VkResultThrowable graphicPlus::AcquireImageOwnership_Presentation(
    VkSemaphore semaphore_renderingIsOver,
    VkSemaphore semaphore_ownershipIsTransfered, VkFence fence) const {
  return impl->AcquireImageOwnership_Presentation(
      semaphore_renderingIsOver, semaphore_ownershipIsTransfered, fence);
}

void graphicPlus::cleanUp() const { impl->CleanUp(); }
