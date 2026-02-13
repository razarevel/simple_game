#include "textures.h"
#include "stbi_image.h"
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <thread>

namespace fs = std::filesystem;

namespace {
uint32_t count = 1;
std::mutex mtx;
}; // namespace
   //
void loadTextures(MAI::Renderer *ren, std::string dir,
                  std::vector<TextureModel> &textures) {
  TextureModel tm;
  std::string name = dir;
  std::replace(name.begin(), name.end(), '\\', '/');
  name = name.substr(name.find_last_of('/') + 1);
  tm.name = name;

  for (const auto &entry : fs::directory_iterator(dir)) {
    std::string str = entry.path();
    int w, h, comp;
    const stbi_uc *pixels = stbi_load(str.c_str(), &w, &h, &comp, 4);
    if (!pixels) {
      std::cerr << "failed to laod texture at " << str << std::endl;
      assert(false);
    }
    MAI::Texture *texture = ren->createImage({
        .type = MAI::TextureType_2D,
        .format = MAI::Format_RGBA_S8,
        .dimensions = {(uint32_t)w, (uint32_t)h},
        .data = pixels,
        .usage = MAI::Sampled_Bit,
    });
    if (str.find("ao") != std::string::npos)
      tm.ao = texture;
    else if (str.find("arm") != std::string::npos)
      tm.arm = texture;
    else if (str.find("diff") != std::string::npos)
      tm.diffuse = texture;
    else if (str.find("disp") != std::string::npos)
      tm.displacement = texture;
    else if (str.find("mask") != std::string::npos)
      tm.mask = texture;
    else if (str.find("nor_dx") != std::string::npos)
      tm.normalDX = texture;
    else if (str.find("nor_gl") != std::string::npos)
      tm.normalGL = texture;
    else if (str.find("rough") != std::string::npos)
      tm.roughness = texture;
    else if (str.find("spec") != std::string::npos)
      tm.specular = texture;
    else {
      std::cerr << str << "not exist" << std::endl;
      assert(false);
    }
  }
  std::lock_guard<std::mutex> lock(mtx);
  tm.id = count;
  textures.emplace_back(tm);
  count++;
}

Textures::Textures(MAI::Renderer *ren) : ren_(ren) {
  std::vector<std::jthread> threads;
  std::string path = RESOURCES_PATH "textures";
  for (const auto &entry : fs::directory_iterator(path)) {
    std::string str = entry.path();
    TextureModel tm;
    threads.emplace_back(loadTextures, ren, str, std::ref(textures));
  }
}
Textures::~Textures() {
  for (auto &it : textures) {
    if (it.ao)
      delete it.ao;
    if (it.arm)
      delete it.arm;
    if (it.diffuse)
      delete it.diffuse;
    if (it.displacement)
      delete it.displacement;
    if (it.mask)
      delete it.mask;
    if (it.normalGL)
      delete it.normalGL;
    if (it.normalDX)
      delete it.normalDX;
    if (it.roughness)
      delete it.roughness;
    if (it.specular)
      delete it.specular;
  }
}
