#pragma once

#include "chunk.hpp"

#include <glm/fwd.hpp>
#include <memory>
#include <unordered_map>

class world {
  struct glm_ivec3_hash {
    std::size_t operator()(const glm::ivec3 &e) const {
      return std::hash<int>()(e.x) ^ std::hash<int>()(e.y) ^
             std::hash<int>()(e.z);
    }
  };

  bool altered = true;

public:
  std::unordered_map<glm::ivec3, std::unique_ptr<chunk>, glm_ivec3_hash> chunks;

  world();
  ~world();

  world(const world &) = delete;
  world &operator=(const world &) = delete;
  world(const world &&) = delete;

  void updateBlockInstanceBuffers();

  inline void initializeWorld() {
    int textureIndex = 0;
    for (int x = -3; x < 4; x++) {
      for (int y = -3; y < 4; y++) {
        for (int z = -3; z < 4; z++) {
          generateChunk(glm::ivec3(x, y, z), textureIndex);
          textureIndex++;
          textureIndex %= 7;
        }
      }
    }
  }
  inline void generateChunk(glm::ivec3 chunkPosition, uint64_t textureIndex) {
    chunks.emplace(chunkPosition, std::make_unique<chunk>(chunkPosition));
    chunks[chunkPosition]->initializeChunk(textureIndex);
  }

  block *getBlock(glm::ivec3 position);
  void setBlock(glm::ivec3 position, block *e);
  void removeBlock(glm::ivec3 position);

  size_t chunkCount() { return chunks.size(); }
};
