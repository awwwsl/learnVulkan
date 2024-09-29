#include "chunk.hpp"

chunk::chunk(glm::ivec3 chunkPosition) : chunkPosition(chunkPosition) {
  blocks = new block *[chunkSize.x * chunkSize.y * chunkSize.z];
  for (int32_t i = 0; i < chunkSize.x * chunkSize.y * chunkSize.z; i++) {
    blocks[i] = nullptr; // HACK: could optimize
  }
#ifndef NDEBUG
  printf("[ chunk ] DEBUG: chunk created at (%d, %d, %d)\n", chunkPosition.x,
         chunkPosition.y, chunkPosition.z);
#endif
}

chunk::~chunk() {
  if (instanceBuffer != VK_NULL_HANDLE) {
    instanceBuffer.~storageBuffer();
  }
  for (int64_t i = 0; i < chunkSize.x * chunkSize.y * chunkSize.z; i++) {
    if (blocks[i] != nullptr) {
      delete blocks[i];
    }
  }
  delete[] blocks;
#ifndef NDEBUG
  printf("[ chunk ] DEBUG: chunk destroyed at (%d, %d, %d,)\n", chunkPosition.x,
         chunkPosition.y, chunkPosition.z);
#endif
}

block *chunk::getBlock(glm::ivec3 position) {
  if (position.x < 0 || position.y < 0 || position.z < 0 ||
      position.x >= chunkSize.x || position.y >= chunkSize.y ||
      position.z >= chunkSize.z) {
#ifndef NDEBUG
    printf("[ chunk ] ERROR: getBlock(position: %d, %d, %d) out of bounds\n",
           position.x, position.y, position.z);
#endif
    return nullptr;
  }
  return blocks[position.x * chunkSize.y * chunkSize.z +
                position.y * chunkSize.z + position.z];
}

void chunk::setBlock(glm::ivec3 position, block *e) {
  if (position.x < 0 || position.y < 0 || position.z < 0 ||
      position.x >= chunkSize.x || position.y >= chunkSize.y ||
      position.z >= chunkSize.z) {
#ifndef NDEBUG
    printf("[ chunk ] ERROR: setBlock(position) out of bounds\n");
#endif
    return;
  }
  if (blocks[position.x * chunkSize.y * chunkSize.z + position.y * chunkSize.z +
             position.z] != nullptr) {
    delete blocks[position.x * chunkSize.y * chunkSize.z +
                  position.y * chunkSize.z + position.z];
    blockCount--;
  }
  blocks[position.x * chunkSize.y * chunkSize.z + position.y * chunkSize.z +
         position.z] = e;
#ifndef NDEBUG
  printf("[ chunk ] DEBUG: setBlock at localPosition(%d, %d, %d) chunk(%d, %d, "
         "%d)\n",
         position.x, position.y, position.z, chunkPosition.x, chunkPosition.y,
         chunkPosition.z);
#endif
  if (e != nullptr) {
    blockCount++;
  }
  altered = true;
}

void chunk::removeBlock(glm::ivec3 position) {
  if (position.x < 0 || position.y < 0 || position.z < 0 ||
      position.x >= chunkSize.x || position.y >= chunkSize.y ||
      position.z >= chunkSize.z) {
#ifndef NDEBUG
    printf("[ chunk ] ERROR: removeBlock(position) out of bounds\n");
#endif
    return;
  }
  if (blocks[position.x * chunkSize.y * chunkSize.z + position.y * chunkSize.z +
             position.z] == nullptr) {
#ifndef NDEBUG
    printf("[ chunk ] ERROR: removeBlock(position) no block at there\n");
#endif
    return;
  }
  delete blocks[position.x * chunkSize.y * chunkSize.z +
                position.y * chunkSize.z + position.z];
  blocks[position.x * chunkSize.y * chunkSize.z + position.y * chunkSize.z +
         position.z] = nullptr;
  blockCount--;
  altered = true;
}
