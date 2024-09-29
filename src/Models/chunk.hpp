#pragma once

#include "Vulkan/storageBuffer.hpp"
#include "block.hpp"
#include "instance.hpp"
#include "textureManager.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

struct glm_ivec3_hash {
  template <typename T> std::size_t operator()(const T &position) const {
    return std::hash<int64_t>()(position.x) ^ std::hash<int64_t>()(position.y) ^
           std::hash<int64_t>()(position.z);
  }
};

class chunk {
  static const constexpr glm::ivec3 initializeChunkSize = {12, 2, 12};
  vulkanWrapper::storageBuffer instanceBuffer = vulkanWrapper::storageBuffer(
      sizeof(instance) * chunkSize.x * chunkSize.y * chunkSize.z);

  bool altered = true;

  block **blocks;
  uint64_t blockCount = 0;

public:
  static const constexpr glm::ivec3 chunkSize = {16, 16, 16};
  const glm::ivec3 chunkPosition;

  chunk(glm::ivec3 chunkPosition);
  ~chunk();

  chunk(const chunk &) = delete;
  chunk(const chunk &&) = delete;
  chunk &operator=(const chunk &) = delete;

  inline block *getBlock(int64_t x, int64_t y, int64_t z) {
    return getBlock(glm::ivec3(x, y, z));
  }
  block *getBlock(glm::ivec3 position);

  // passing ownership
  inline void setBlock(int64_t x, int64_t y, int64_t z, block *e) {
    return setBlock(glm::ivec3(x, y, z), e);
  }
  // passing ownership
  void setBlock(glm::ivec3 position, block *e);

  inline void removeBlock(int64_t x, int64_t y, int64_t z) {
    return removeBlock(glm::ivec3(x, y, z));
  }
  void removeBlock(glm::ivec3 position);

  inline void initializeChunk(uint32_t textureIndex) {
    int64_t x = initializeChunkSize.x, y = initializeChunkSize.y,
            z = initializeChunkSize.z;
    for (int64_t ix = 0; ix < x; ix++) {
      for (int64_t iy = 0; iy < y; iy++) {
        for (int64_t iz = 0; iz < z; iz++) {
          block *b = new block(
              glm::ivec3(ix, iy, iz) + chunkPosition * chunkSize, textureIndex);
          setBlock(ix, iy, iz, b);
        }
      }
    }
  }

  inline std::vector<instance> getInstances() {
    std::vector<instance> instances;
    instances.reserve(chunkSize.x * chunkSize.y * chunkSize.z);
    for (int64_t ix = 0; ix < chunkSize.x; ix++)
      for (int64_t iy = 0; iy < chunkSize.y; iy++)
        for (int64_t iz = 0; iz < chunkSize.z; iz++) {
          int64_t index =
              ix * chunkSize.y * chunkSize.z + iy * chunkSize.z + iz;
          struct instance inst =
              blocks[index] == nullptr
                  ? instance{.isValid = false}
                  : instance{.model = blocks[index]->getModelMatrix(),
                             .textureIndex = blocks[index]->textureIndex,
                             .isValid = true};
          instances.push_back(inst);
        };
    return instances;
  }

  inline void updateChunkBuffer() {
    if (!altered)
      return;
    instanceBuffer.TransferData(getInstances().data(),
                                sizeof(instance) * chunkSize.x * chunkSize.y *
                                    chunkSize.z);
    altered = false;
  }

  inline VkDescriptorBufferInfo descriptorBufferInfo() {
    return VkDescriptorBufferInfo{
        .buffer = instanceBuffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE,
    };
  }

  inline uint64_t size() { return blockCount; }
};
