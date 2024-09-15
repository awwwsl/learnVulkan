#pragma once
#include <vulkan/vulkan_format_traits.hpp>

struct formatInfo {
  enum rawDataType : uint8_t {
    other,        // 0，没数据或各个通道不同
    integer,      // 1，数据类型为整型
    floatingPoint // 1，数据类型为浮点数
  };
  uint8_t componentCount; // 通道数
  uint8_t sizePerComponent; // 每个通道的大小，0意味着压缩，或不均等，或少于1
  uint8_t sizePerPixel; // 每个像素的大小，0意味着压缩
  uint8_t rawDataType;  // 底层数据类型

  static const formatInfo FormatInfo(VkFormat format);
  // static const formatInfo getSizeMemory(VkFormat format);
  // static const formatInfo getSizeFileSystem(VkFormat format);
};
