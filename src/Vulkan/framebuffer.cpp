#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

#include "framebuffer.hpp"

#include "../Models/graphic.hpp"

#include <stdio.h>

VkResultThrowable
vulkanWrapper::framebuffer::Create(VkFramebufferCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  VkResult result = vkCreateFramebuffer(graphic::Singleton().Device(),
                                        &createInfo, nullptr, &handle);
  if (result)
    printf("[ framebuffer ] ERROR: Failed to create a "
           "framebuffer!\nError code: %d\n",
           int32_t(result));
#ifndef NDEBUG
  printf("[ framebuffer ] DEBUG: Framebuffer created with handle %p\n",
         (void *)handle);
#endif
  return result;
}

vulkanWrapper::framebuffer::framebuffer() {}

vulkanWrapper::framebuffer::framebuffer(framebuffer &&other) noexcept {
  MoveHandle;
}
vulkanWrapper::framebuffer::~framebuffer() {
  DestroyHandleBy(vkDestroyFramebuffer, "framebuffer");
}
