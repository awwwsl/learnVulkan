#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include "../Utils/Macros.hpp"
#include "../Utils/VkResultThrowable.hpp"

#include "framebuffer.hpp"

#include "../Models/graphic.hpp"

#include <stdio.h>

VkResultThrowable
vulkanWrapper::framebuffer::Create(VkFramebufferCreateInfo &createInfo) {
  this->size = {createInfo.width, createInfo.height};
  createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  VkResult result = vkCreateFramebuffer(graphic::Singleton().Device(),
                                        &createInfo, nullptr, &handle);
  if (result)
    printf("[ framebuffer ] ERROR: Failed to create a "
           "framebuffer!\nError code: %d\n",
           int32_t(result));
#ifndef NDEBUG
  ThreadId(id);
  printf("[ framebuffer ] [Thread: %s] DEBUG: Framebuffer created with handle "
         "%p\n",
         id.c_str(), (void *)handle);
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
