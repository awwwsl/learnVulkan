#include "graphicsPipelineCreateInfoPack.hpp"

graphicsPipelineCreateInfoPack::graphicsPipelineCreateInfoPack() {
  SetCreateInfos();
  // 若非派生管线，createInfo.basePipelineIndex不得为0，设置为-1
  createInfo.basePipelineIndex = -1;
}
// 移动构造器，所有指针都要重新赋值
graphicsPipelineCreateInfoPack::graphicsPipelineCreateInfoPack(
    const graphicsPipelineCreateInfoPack &other) noexcept {
  createInfo = other.createInfo;
  SetCreateInfos();

  vertexInputStateCi = other.vertexInputStateCi;
  inputAssemblyStateCi = other.inputAssemblyStateCi;
  tessellationStateCi = other.tessellationStateCi;
  viewportStateCi = other.viewportStateCi;
  rasterizationStateCi = other.rasterizationStateCi;
  multisampleStateCi = other.multisampleStateCi;
  depthStencilStateCi = other.depthStencilStateCi;
  colorBlendStateCi = other.colorBlendStateCi;
  dynamicStateCi = other.dynamicStateCi;

  shaderStages = other.shaderStages;
  vertexInputBindings = other.vertexInputBindings;
  vertexInputAttributes = other.vertexInputAttributes;
  viewports = other.viewports;
  scissors = other.scissors;
  colorBlendAttachmentStates = other.colorBlendAttachmentStates;
  dynamicStates = other.dynamicStates;
  UpdateAllArrayAddresses();
}
// Getter，这里我没用const修饰符

// Non-const Function
// 该函数用于将各个vector中数据的地址赋值给各个创建信息中相应成员，并相应改变各个count
void graphicsPipelineCreateInfoPack::UpdateAllArrays() {
  createInfo.stageCount = shaderStages.size();
  vertexInputStateCi.vertexBindingDescriptionCount = vertexInputBindings.size();
  vertexInputStateCi.vertexAttributeDescriptionCount =
      vertexInputAttributes.size();
  viewportStateCi.viewportCount =
      viewports.size() ? uint32_t(viewports.size()) : dynamicViewportCount;
  viewportStateCi.scissorCount =
      scissors.size() ? uint32_t(scissors.size()) : dynamicScissorCount;
  colorBlendStateCi.attachmentCount = colorBlendAttachmentStates.size();
  dynamicStateCi.dynamicStateCount = dynamicStates.size();
  UpdateAllArrayAddresses();
}

// 该函数用于将创建信息的地址赋值给basePipelineIndex中相应成员
void graphicsPipelineCreateInfoPack::SetCreateInfos() {
  createInfo.pVertexInputState = &vertexInputStateCi;
  createInfo.pInputAssemblyState = &inputAssemblyStateCi;
  createInfo.pTessellationState = &tessellationStateCi;
  createInfo.pViewportState = &viewportStateCi;
  createInfo.pRasterizationState = &rasterizationStateCi;
  createInfo.pMultisampleState = &multisampleStateCi;
  createInfo.pDepthStencilState = &depthStencilStateCi;
  createInfo.pColorBlendState = &colorBlendStateCi;
  createInfo.pDynamicState = &dynamicStateCi;
}
// 该函数用于将各个vector中数据的地址赋值给各个创建信息中相应成员，但不改变各个count
void graphicsPipelineCreateInfoPack::UpdateAllArrayAddresses() {
  createInfo.pStages = shaderStages.data();
  vertexInputStateCi.pVertexBindingDescriptions = vertexInputBindings.data();
  vertexInputStateCi.pVertexAttributeDescriptions =
      vertexInputAttributes.data();
  viewportStateCi.pViewports = viewports.data();
  viewportStateCi.pScissors = scissors.data();
  colorBlendStateCi.pAttachments = colorBlendAttachmentStates.data();
  dynamicStateCi.pDynamicStates = dynamicStates.data();
};
