#pragma once

#include <assimp/scene.h>
#include <glm/glm.hpp>

struct AssimpToGlm {
  static glm::mat4 transToMat4(aiMatrix4x4 mat) {
    return glm::mat4(mat.a1, mat.a2, mat.a3, mat.a4, mat.b1, mat.b2, mat.b3,
                     mat.b4, mat.c1, mat.c2, mat.c3, mat.c4, mat.d1, mat.d2,
                     mat.d3, mat.d4);
  }
};
