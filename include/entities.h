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

enum ActionType : uint8_t {
  ENTITY = 0,
  ADD = 1,
};

struct EntityData {
  TextureModel *tm = nullptr;
  bool disable = false;
  glm::vec3 pos = glm::vec3(0.0f);
  glm::vec3 scale = glm::vec3(1.0f);
  glm::vec3 rotate = glm::vec3(0.0f);
};

struct Entity {
  uint32_t id;
  std::string name;
  EntityType type;
  EntityData entityData;
};

struct Action {
  ActionType type;
  uint32_t id;
  EntityData data;
};

struct Entities {
  Entities(MAI::Renderer *ren, GLFWwindow *window, VkFormat formt);
  ~Entities();

  void guiWidget();

  void entityWidget();

  void draw(MAI::CommandBuffer *buff, glm::mat4 proj, glm::mat4 view,
            glm::vec3 cameraPos);

  void undoCheck();

private:
  GLFWwindow *window;
  MAI::Renderer *ren_;
  VkFormat format;
  MAI::Pipeline *pipeline_;
  Assets *assets;
  Textures *textures;
  Shapes *shapes;
  uint32_t currentEntity = -1;

  int currAction = -1;
  std::vector<Action> actions;
  std::vector<Entity> entities;

  void preparePipelines();
  void actionAdd(uint32_t id, ActionType type = ADD, EntityData data = {});
};
