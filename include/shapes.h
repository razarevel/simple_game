#pragma once

#include "mai_config.h"
#include "mai_vk.h"
#include <string>
#include <vector>

#include <glm/glm.hpp>

struct ShapeModule {
  uint32_t id;
  std::string name;
  MAI::Buffer *vertBuff = nullptr;
  MAI::Buffer *indexBuff = nullptr;
  uint32_t indicesSize;
};

struct Shapes {
  Shapes(MAI::Renderer *ren, VkFormat format);
  ~Shapes();

  const std::vector<ShapeModule> &getShapesInfo() const { return shapes; }

  ShapeModule *getShapeModule(uint32_t id) {
    for (auto &sp : shapes) {
      if (sp.id == id)
        return &sp;
    }
    return nullptr;
  }

private:
  MAI::Renderer *ren_;
  uint32_t shapeCount = 0;
  std::vector<ShapeModule> shapes;

  ShapeModule prepareShape(const std::vector<glm::vec4> &vertices,
                           const std::vector<uint16_t> &indices,
                           std::string name);
};
