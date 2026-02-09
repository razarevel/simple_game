#pragma once
#include "mai_config.h"
#include "mai_vk.h"
#include "model.h"

struct ModelInfo {
  uint32_t id;
  std::string name;
};

struct Entities {
  Entities(MAI::Renderer *ren, VkFormat format);
  ~Entities();

  void drawModels(MAI::CommandBuffer *buffer, glm::mat4 proj, glm::mat4 view,
                  uint32_t id);

  std::vector<ModelInfo> getModelInfos();

private:
  MAI::Renderer *ren_;
  std::vector<Model *> models;
  MAI::Pipeline *pipeline_ = nullptr;

  void preparePipeline(VkFormat format);
};
