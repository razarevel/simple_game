#pragma once
#include "assets.h"
#include "imgui.h"
#include "mai_config.h"
#include "mai_vk.h"
#include "shapes.h"
#include "textures.h"
#include "utils.h"

enum EntityType : uint8_t {
  ASSET = 0,
  SHAPE = 1,
};

enum ActionType : uint8_t {
  ENTITY = 0,
  ADD = 1,
};

struct EntityData {
  uint32_t textureId;
  bool disable = false;
  float tiling = 0.1f;
  glm::vec3 pos = glm::vec3(0.0f);
  glm::vec3 scale = glm::vec3(1.0f);
  glm::vec3 rotate = glm::vec3(0.0f);
};

struct Entity {
  uint32_t id;
  uint32_t addId;
  EntityType type;
  EntityData entityData;
};

struct Action {
  ActionType type;
  uint32_t id;
  EntityData data;
};

struct EntityDrawInfo {
  MAI::CommandBuffer *buff;
  glm::mat4 proj;
  glm::mat4 view;
  glm::vec3 cameraPos;
  MouseState mouse_state;
};

struct Entities {
  Entities(MAI::Renderer *ren, GLFWwindow *window, VkFormat formt);
  ~Entities();

  void guiWidget();
  void entityWidget();
  void draw(EntityDrawInfo info);
  void undoCheck();
  void saveEntity();
  void resetEntity();
  void loadEntity();

private:
  GLFWwindow *window;
  MAI::Renderer *ren_;
  VkFormat format;
  MAI::Pipeline *pipeline_;
  MAI::Pipeline *ShapePipeline_;
  Assets *assets;
  Textures *textures;
  Shapes *shapes;
  uint32_t currentEntity = -1;
  EntityDrawInfo drawInfo_;

  int currAction = -1;
  std::vector<Action> actions;
  std::vector<Entity> entities;

  void preparePipelines();
  void actionAdd(uint32_t id, ActionType type = ADD, EntityData data = {});
  void checkMouseClick();
};
