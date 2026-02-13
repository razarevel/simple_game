#pragma once
#include "mai_config.h"
#include "mai_vk.h"
#include "model.h"

struct ModelInfo {
  uint32_t id;
  std::string name;
};

struct Assets {
  Assets(MAI::Renderer *ren, VkFormat format);
  ~Assets();

  // void drawModels(MAI::CommandBuffer *buffer, glm::mat4 proj, glm::mat4 view,
  //                 uint32_t id);

  std::vector<ModelInfo> getModelInfos();

  Model *getModel(uint32_t id) {
    Model *model = nullptr;
    for (auto &it : models)
      if (it->id == id)
        return it;
    assert(false);
  }

private:
  MAI::Renderer *ren_;
  std::vector<Model *> models;
  MAI::Pipeline *pipeline_ = nullptr;

  void preparePipeline(VkFormat format);
};
