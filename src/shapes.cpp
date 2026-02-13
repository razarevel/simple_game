#include "shapes.h"
#include <imgui.h>

std::vector<glm::vec4> cubeVertices = {

    // Front (+Z) → 0
    {-0.5f, -0.5f, 0.5f, 0.0f},
    {0.5f, -0.5f, 0.5f, 0.0f},
    {0.5f, 0.5f, 0.5f, 0.0f},
    {-0.5f, 0.5f, 0.5f, 0.0f},

    // Back (-Z) → 1
    {0.5f, -0.5f, -0.5f, 1.0f},
    {-0.5f, -0.5f, -0.5f, 1.0f},
    {-0.5f, 0.5f, -0.5f, 1.0f},
    {0.5f, 0.5f, -0.5f, 1.0f},

    // Left (-X) → 2
    {-0.5f, -0.5f, -0.5f, 2.0f},
    {-0.5f, -0.5f, 0.5f, 2.0f},
    {-0.5f, 0.5f, 0.5f, 2.0f},
    {-0.5f, 0.5f, -0.5f, 2.0f},

    // Right (+X) → 3
    {0.5f, -0.5f, 0.5f, 3.0f},
    {0.5f, -0.5f, -0.5f, 3.0f},
    {0.5f, 0.5f, -0.5f, 3.0f},
    {0.5f, 0.5f, 0.5f, 3.0f},

    // Top (+Y) → 4
    {-0.5f, 0.5f, 0.5f, 4.0f},
    {0.5f, 0.5f, 0.5f, 4.0f},
    {0.5f, 0.5f, -0.5f, 4.0f},
    {-0.5f, 0.5f, -0.5f, 4.0f},

    // Bottom (-Y) → 5
    {-0.5f, -0.5f, -0.5f, 5.0f},
    {0.5f, -0.5f, -0.5f, 5.0f},
    {0.5f, -0.5f, 0.5f, 5.0f},
    {-0.5f, -0.5f, 0.5f, 5.0f},
};

std::vector<uint16_t> cubeIndices = {
    0,  1,  2,  2,  3,  0,  // Front
    4,  5,  6,  6,  7,  4,  // Back
    8,  9,  10, 10, 11, 8,  // Left
    12, 13, 14, 14, 15, 12, // Right
    16, 17, 18, 18, 19, 16, // Top
    20, 21, 22, 22, 23, 20  // Bottom
};

ShapeModule Shapes::prepareShape(const std::vector<glm::vec4> &vertices,
                                 const std::vector<uint16_t> &indices,
                                 std::string name) {

  ShapeModule shape;

  shape.vertBuff = ren_->createBuffer({
      .usage = MAI::StorageBuffer,
      .storage = MAI::StorageType_Device,
      .size = sizeof(glm::vec4) * cubeVertices.size(),
      .data = cubeVertices.data(),
  });

  shape.indexBuff = ren_->createBuffer({
      .usage = MAI::IndexBuffer,
      .storage = MAI::StorageType_Device,
      .size = sizeof(uint16_t) * cubeIndices.size(),
      .data = cubeIndices.data(),
  });
  shape.indicesSize = (uint32_t)cubeIndices.size();

  shape.name = name;
  shape.id = shapeCount;

  shapeCount++;
  return shape;
}

Shapes::Shapes(MAI::Renderer *ren, VkFormat format) : ren_(ren) {

  shapes.emplace_back(prepareShape(cubeVertices, cubeIndices, "cube"));
}

Shapes::~Shapes() {
  for (auto &it : shapes) {
    delete it.vertBuff;
    delete it.indexBuff;
  }
}
