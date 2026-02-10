#pragma once
#include "assets.h"
#include "mai_config.h"
#include "mai_vk.h"
#include "shapes.h"
#include "textures.h"

enum EntityType : uint8_t {
  ASSET = 0,
  SHAPE = 1,
};

struct Entity {
  uint32_t id;
  std::string name;
  EntityType type;
  glm::vec3 pos = glm::vec3(0.0f);
  glm::vec3 scale = glm::vec3(0.0f);
  glm::vec3 rotate = glm::vec3(0.0f);
};

struct Entities {
  Entities(MAI::Renderer *ren, VkFormat formt);
  ~Entities();

  void guiWidget();

  void entityWidget();

  void draw(MAI::CommandBuffer *buff, glm::mat4 proj, glm::mat4 view,
            glm::vec3 cameraPos);

private:
  MAI::Renderer *ren_;
  VkFormat format;
  MAI::Pipeline *pipeline_;
  Assets *assets;
  Textures *textures;
  Shapes *shapes;
  uint32_t currentEntity = -1;

  std::vector<Entity> entities;

  void preparePipelines();
};
