#include "maiApp.h"
#include "skybox.h"
#include <fonts.h>

int main() {
  MaiApp *mai = new MaiApp();

  Skybox *skybox = new Skybox(mai->ren, mai->depthTexture);
  FontRenderer *fmt =
      new FontRenderer(mai->ren, mai->windowInfo.width, mai->windowInfo.height);

  auto draw = [&](MAI::CommandBuffer *buff, uint32_t width, uint32_t height,
                  float ratio, float deltaSecond) {
    glm::mat4 p = glm::perspective(glm::radians(60.0f), ratio, 0.1f, 1000.0f);
    p[1][1] *= -1;
    skybox->draw(buff, ratio, p, mai->camera->getViewMatrix(),
                 mai->camera->getPosition());
    fmt->draw(buff);
  };

  auto beforeDraw = [&](MAI::CommandBuffer *buff, uint32_t width,
                        uint32_t height, float ratio, float deltaSecond) {
    std::string currentFPS = std::to_string((uint32_t)mai->currentFPS);
    fmt->drawDynamicText(buff, currentFPS.c_str(), "fps",
                         glm::vec2(0.9f, 0.01f), glm::vec3(0, 0.98, 0.863));
  };

  auto afterDraw = [&](MAI::CommandBuffer *buff, uint32_t width,
                       uint32_t height, float ratio,
                       float deltaSecond) { fmt->clearGarbge(); };

  mai->run(draw, beforeDraw, afterDraw);

  delete fmt;

  delete skybox;
  delete mai;

  return 0;
}
