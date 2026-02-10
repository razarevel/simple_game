#pragma once
#include "mai_config.h"
#include "mai_vk.h"
#include <string>
#include <vector>

struct TextureModel {
  uint32_t id;
  std::string name;
  MAI::Texture *ao = nullptr;
  MAI::Texture *arm = nullptr; // AO/Rough/Material
  MAI::Texture *diffuse = nullptr;
  MAI::Texture *displacement = nullptr;
  MAI::Texture *mask = nullptr;
  MAI::Texture *normalDX = nullptr;
  MAI::Texture *normalGL = nullptr;
  MAI::Texture *roughness = nullptr;
  MAI::Texture *specular = nullptr;
};

struct Textures {
  Textures(MAI::Renderer *ren);
  ~Textures();
  std::vector<TextureModel> &getTextures() { return textures; }

private:
  MAI::Renderer *ren_;
  std::vector<TextureModel> textures;
};
