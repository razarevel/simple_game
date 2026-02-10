#include "imgui.h"
#include <Bitmap.h>
#include <UtilsCubemap.h>
#include <filesystem>
#include <mutex>
#include <skybox.h>
#include <thread>

namespace fs = std::filesystem;

#include "stbi_image.h"

namespace {
uint32_t count = 0;
std::mutex mtx;
}; // namespace

void loadCubemape(MAI::Renderer *ren, std::string dir,
                  std::vector<Cubemap> &cubemaps) {
  int w, h;
  const float *img = stbi_loadf(dir.c_str(), &w, &h, nullptr, 4);
  assert(img);
  Bitmap in(w, h, 4, eBitmapFormat_Float, img);
  Bitmap out = convertEquirectangularMapToVerticalCross(in);
  stbi_image_free((void *)img);

  Bitmap cubemap = convertVerticalCrossToCubeMapFaces(out);

  MAI::Texture *cube = ren->createImage({
      .type = MAI::TextureType_Cube,
      .format = MAI::Format_RGBA_F32,
      .dimensions = {(uint32_t)cubemap.w_, (uint32_t)cubemap.h_},
      .data = cubemap.data_.data(),
      .usage = MAI::Sampled_Bit,
  });

  std::string name = dir;
  name = name.substr(name.find_last_of('/') + 1);
  std::size_t pos = name.find_last_of('.');
  name = name.substr(0, pos);

  std::lock_guard<std::mutex> lock(mtx);
  cubemaps.emplace_back(Cubemap{
      .name = name,
      .id = count,
      .tex = cube,
  });
  count++;
}

Skybox::Skybox(MAI::Renderer *ren, VkFormat format) : ren_(ren) {
  MAI::Shader *vert_ = ren_->createShader(SHADERS_PATH "spvs/skybox.vspv");
  MAI::Shader *frag_ = ren_->createShader(SHADERS_PATH "spvs/skybox.fspv");
  pipeline_ = ren_->createPipeline({
      .vert = vert_,
      .frag = frag_,
      .depthFormat = format,
  });
  delete vert_;
  delete frag_;

  std::vector<std::jthread> threads;
  std::string path = RESOURCES_PATH "skybox";
  for (const auto &entry : fs::directory_iterator(path)) {
    std::string str = entry.path();
    threads.emplace_back(loadCubemape, ren, str, std::ref(cubemaps));
  }

  assert(threads.size() != 0);
}

void Skybox::draw(const DrawInfo &info) {

  struct PushConstant {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
    uint32_t textureId;
  } pc{
      .view = info.view,
      .proj = info.proj,
      .cameraPos = glm::vec4(info.cameraPos, 1.0f),
      .textureId = cubemaps[currSkybox.back()].tex->getIndex(),
  };
  info.buff->bindPipeline(pipeline_);
  info.buff->cmdPushConstant(&pc);
  info.buff->cmdDraw(36);
}

void Skybox::guiWidgets() {
  if (ImGui::TreeNode("Skybox")) {
    for (auto it : cubemaps)
      if (ImGui::Button(it.name.c_str()))
        currSkybox.push_back(it.id);
    ImGui::TreePop();
  }
}

Skybox::~Skybox() {
  for (auto &it : cubemaps)
    delete it.tex;
  cubemaps.clear();
  delete pipeline_;
}
