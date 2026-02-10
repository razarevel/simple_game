#pragma once

#include "mai_config.h"
#include "mai_vk.h"
#include <map>
#include <string>

#include <glm/glm.hpp>

struct ShapeModule {
  MAI::Buffer *vertBuff = nullptr;
  MAI::Buffer *indexBuff = nullptr;
  uint32_t indicesSize;
};

struct ShapeVertex {
  glm::vec3 pos;
  glm::vec2 uv;
};

struct Shapes {
  Shapes(MAI::Renderer *ren, VkFormat format);
  ~Shapes();

  void drawShapes(MAI::CommandBuffer *buff, glm::mat4 proj, glm::mat4 view);

  std::vector<std::string> getShapesInfo() {
    std::vector<std::string> shapesInfo;
    shapesInfo.reserve(shapes.size());
    for (auto &it : shapes)
      shapesInfo.emplace_back(it.first);
    return shapesInfo;
  }

  ShapeModule *getShapeModule(std::string name) {
    auto it = shapes.find(name);
    assert(it != shapes.end());
    return &it->second;
  }

private:
  MAI::Renderer *ren_;
  MAI::Pipeline *pipeline_;

  uint32_t shapeCount = 0;

  std::map<std::string, ShapeModule> shapes;

  ShapeModule prepareShape(const std::vector<ShapeVertex> &vertices,
                           const std::vector<uint16_t> &indices);
};
