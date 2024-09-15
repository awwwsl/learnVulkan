
#include "pipelineLayout.hpp"
#include "graphic.hpp"

#include <stdio.h>

VkResultThrowable
vulkanWrapper::pipelineLayout::Create(VkPipelineLayoutCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  VkResult result = vkCreatePipelineLayout(graphic::Singleton().Device(),
                                           &createInfo, nullptr, &handle);
  if (result)
    printf("[ pipelineLayout ] ERROR: Failed to create a "
           "pipeline layout!\nError code: %d\n",
           int32_t(result));
  return result;
}

vulkanWrapper::pipelineLayout::~pipelineLayout() {
  DestroyHandleBy(vkDestroyPipelineLayout, "pipelineLayout")
}
