#include "assets.h"
#include <filesystem>
#include <mutex>
#include <thread>
namespace fs = std::filesystem;

namespace {
uint32_t count = 0;
std::mutex mtx;
}; // namespace

void loadModel(MAI::Renderer *ren, std::string dir,
               std::vector<Model *> &models) {
  Model *md = new Model(ren, dir.c_str());
  std::lock_guard<std::mutex> lock(mtx);
  md->id = count;
  models.emplace_back(md);
  count++;
}

Assets::Assets(MAI::Renderer *ren, VkFormat format) : ren_(ren) {

  // assets load
  std::vector<std::jthread> threads;
  std::string path = RESOURCES_PATH "assets";
  for (const auto &entry : fs::directory_iterator(path)) {
    std::string str = entry.path();
    threads.emplace_back(loadModel, ren, str, std::ref(models));
  }
  preparePipeline(format);
}

// void Assets::drawModels(MAI::CommandBuffer *buffer, glm::mat4 proj,
//                         glm::mat4 view, uint32_t id) {
//   buffer->bindPipeline(pipeline_);
//   for (auto &it : models) {
//     if (it->id == id)
//       it->draw(buffer, proj, view, glm::mat4(1.0f));
//   }
// }

std::vector<ModelInfo> Assets::getModelInfos() {
  std::vector<ModelInfo> infos;
  for (auto &it : models)
    infos.emplace_back(ModelInfo{
        .id = it->id,
        .name = it->name,
    });
  return infos;
}

Assets::~Assets() {
  for (auto &it : models)
    delete it;
  delete pipeline_;
}

void Assets::preparePipeline(VkFormat format) {
  MAI::Shader *vert = ren_->createShader(SHADERS_PATH "spvs/model.vspv");
  MAI::Shader *frag = ren_->createShader(SHADERS_PATH "spvs/model.fspv");
  pipeline_ = ren_->createPipeline({
      .vert = vert,
      .frag = frag,
      .depthFormat = format,
      .cullMode = MAI::CullMode::Back,
  });
  delete vert;
  delete frag;
}
