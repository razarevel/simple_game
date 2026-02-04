#include <Bitmap.h>
#include <UtilsCubemap.h>
#include <skybox.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Skybox::Skybox(MAI::Renderer *ren, MAI::Texture *depthTexture) : ren_(ren) {
  MAI::Shader *vert_ = ren_->createShader(SHADERS_PATH "skybox.vert");
  MAI::Shader *frag_ = ren_->createShader(SHADERS_PATH "skybox.frag");
  pipeline_ = ren_->createPipeline({
      .vert = vert_,
      .frag = frag_,
      .depthFormat = depthTexture->getDeptFormat(),
  });
  delete vert_;
  delete frag_;

  {
    int w, h;
    const float *img =
        stbi_loadf(RESOURCES_PATH "skybox/qwantani_moonrise_puresky.hdr", &w,
                   &h, nullptr, 4);
    assert(img);
    Bitmap in(w, h, 4, eBitmapFormat_Float, img);
    Bitmap out = convertEquirectangularMapToVerticalCross(in);
    stbi_image_free((void *)img);

    Bitmap cubemap = convertVerticalCrossToCubeMapFaces(out);

    cubemapTex = ren->createImage({
        .type = MAI::TextureType_Cube,
        .format = MAI::Format_RGBA_F32,
        .dimensions = {(uint32_t)cubemap.w_, (uint32_t)cubemap.h_},
        .data = cubemap.data_.data(),
        .usage = MAI::Sampled_Bit,
    });
  }
}

void Skybox::draw(MAI::CommandBuffer *buff, float ratio, glm::mat4 proj,
                  glm::mat4 view, glm::vec3 cameraPos) {

  struct PushConstant {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPos;
    uint32_t textureId;
  } pc{
      .view = view,
      .proj = proj,
      .cameraPos = glm::vec4(cameraPos, 1.0f),
      .textureId = cubemapTex->getIndex(),
  };
  buff->bindPipeline(pipeline_);
  buff->cmdPushConstant(&pc);
  buff->cmdDraw(36);
}

Skybox::~Skybox() {
  delete cubemapTex;
  delete pipeline_;
}
