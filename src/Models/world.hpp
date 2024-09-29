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

  world(const char *path);

  world(const world &) = delete;
  world &operator=(const world &) = delete;
  world(const world &&) = delete;

  void updateBlockInstanceBuffers();

  block *getBlock(glm::ivec3 position);
  void setBlock(glm::ivec3 position, block *e);
  void removeBlock(glm::ivec3 position);

  size_t chunkCount() { return chunks.size(); }
};
