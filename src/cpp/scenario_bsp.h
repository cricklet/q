#ifndef SCENARIO_BSP_H
#define SCENARIO_BSP_H

#include "scenario.h"

struct Camera {
public:
  glm::vec3 location;
  double pitch;
  double yaw;

  glm::vec3 forward();
  glm::vec3 right();
  glm::vec3 upApproximate();
};

struct RenderableBSP;

struct SceneShaderParameters {
  GLuint inPosition;
  GLuint inColor;
  GLuint inTextureCoords;
  GLuint inLightmapCoords;

  GLuint unifHighlight;
  GLuint unifAlpha;
  GLuint unifTexture;
  GLuint unifLightmapTexture;

  GLuint unifCameraTransform;
  GLuint unifProjTransform;
};

struct BSPScenario : IScenario {
public:
  BSPScenario();
  void think(glm::vec2 dir, double pitch, double yaw) override;
  void render() override;

private:
  bool finishLoading() override;

  int _bspResourceID;
  int _sceneShaderResourceID;

  shared_ptr<TextureRenderer> _compositingRenderer = nullptr;
  shared_ptr<RenderableBSP> _renderableMap = nullptr;

  unordered_map<int, GLuint> _lightmapTextures;
  GLuint _fallbackLightmapTexture;

  GLuint _sceneShader;
  SceneShaderParameters _sceneShaderParams;

  GLuint _vao;
  
  // For rendering with the scene shader
  vector<VBO> _verticesPerFace;
  vector<VBO> _colorsPerFace;
  vector<EBO> _elementsPerFace;

  // For rendering solid elements
  GLuint _sceneFBO;
  GLuint _sceneTexture;
  GLuint _sceneDepthTexture;

  // For rendering transparency
  GLuint _effectsFBO;
  GLuint _effectsTexture;

  // For rendering FBOs to the screen
  VBO _screenVBO;
  GLuint _screenShader;
  
  // For testing
  int _poptartResourceID;

  Camera _camera;
};

#endif
