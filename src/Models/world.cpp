#include "world.hpp"

world::world() = default;
world::~world() = default;

//  0  -  15 -> chunk 0
// -16 - -1  -> chunk -1

glm::ivec3 absMod(glm::ivec3 a, glm::ivec3 b) { return (a % b + b) % b; }
glm::ivec3 smallDiv(glm::ivec3 a, glm::ivec3 b) {
  auto div = [](int a, int b) {
    if (a < 0)
      return (a + 1) / b - 1;
    return a / b;
  };

  return glm::ivec3(div(a.x, b.x), div(a.y, b.y), div(a.z, b.z));
}

block *world::getBlock(glm::ivec3 position) {
  glm::ivec3 localPosition = absMod(position, chunk::chunkSize);
  auto find = chunks.find(smallDiv(position, chunk::chunkSize));
  if (find == chunks.end())
    return nullptr;
  return find->second->getBlock(localPosition);
}

void world::setBlock(glm::ivec3 position, block *e) {
  glm::ivec3 localPosition = absMod(position, chunk::chunkSize);
  auto find = chunks.find(smallDiv(position, chunk::chunkSize));
  if (find == chunks.end()) {
    chunks
        .insert({smallDiv(position, chunk::chunkSize),
                 std::make_unique<chunk>(smallDiv(position, chunk::chunkSize))})
        .first->second->setBlock(localPosition, e);
    altered = true;
  } else {
    find->second->setBlock(localPosition, e);
  }
}

void world::removeBlock(glm::ivec3 position) {
  glm::ivec3 localPosition = absMod(position, chunk::chunkSize);
  auto find = chunks.find(smallDiv(position, chunk::chunkSize));
  if (find == chunks.end()) {
#ifndef NDEBUG
    printf("[ world ] ERROR: Tried to remove block from non-existent chunk: "
           "position(%d, %d, %d)\n",
           position.x, position.y, position.z);
#endif
    return;
  }
  find->second->removeBlock(localPosition);
  if (find->second->size() == 0)
    chunks.erase(find);
  altered = true;
}

void world::updateBlockInstanceBuffers() {
  for (auto &chunk : chunks) {
    chunk.second->updateChunkBuffer();
  }
}
