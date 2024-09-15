#include "pipeline.hpp"
#include "graphic.hpp"

#include <stdio.h>

VkResultThrowable
vulkanWrapper::pipeline::Create(VkGraphicsPipelineCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  VkResult result =
      vkCreateGraphicsPipelines(graphic::Singleton().Device(), VK_NULL_HANDLE,
                                1, &createInfo, nullptr, &handle);
  if (result)
    printf("[ pipeline ] ERROR: Failed to create a "
           "graphics pipeline!\nError code: %d\n",
           int32_t(result));
  return result;
}
VkResultThrowable
vulkanWrapper::pipeline::Create(VkComputePipelineCreateInfo &createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  VkResult result =
      vkCreateComputePipelines(graphic::Singleton().Device(), VK_NULL_HANDLE, 1,
                               &createInfo, nullptr, &handle);
  if (result)
    printf("[ pipeline ] ERROR: Failed to create a compute "
           "pipeline!\nError code: %d\n",
           int32_t(result));
  return result;
}

vulkanWrapper::pipeline::pipeline(pipeline &&other) noexcept { MoveHandle; }
vulkanWrapper::pipeline::~pipeline() {
  DestroyHandleBy(vkDestroyPipeline, "pipeline");
}
