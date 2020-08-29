#include "renderable.h"

#include "bsp.h"
#include "gl_helpers.h"
#include "resource_manager.h"
#include "scenario.h"
#include "scenario_bsp.h"
#include "hitscan.h"
#include "pprint.hpp"
#include "assert.h"

RenderableBSP::RenderableBSP(ResourcePtr<const BSPMap> mapPtr): _map(mapPtr) {
  assert(_map);

  const BSP::texture_t* textures = _map->textures();
  const int numTextures = _map->numTextures();

  int textureResourceId = 100;

  for (int textureIndex = 0; textureIndex < numTextures; textureIndex ++) {
    const BSP::texture_t* texture = textures + textureIndex;
    
    _textureResourceIds[string(texture->name)] = textureResourceId;
    ResourceManager::getInstance()->loadResource(this, {
      string("./data/") + string(texture->name),
      ResourceType::IMAGE_FILE,
      textureResourceId
    });

    textureResourceId ++;
  }
}


bool RenderableBSP::finishLoading() {
  const BSPMap* map = _map.get();
  if (!map) {
    cerr << "map failed to load\n";
    return false;
  }

  map->print();
  // map->printEffects();
  // map->printTextures();
  // map->printVertices();
  // map->printFaces();
  // map->printMeshverts();

  { // Load lightmaps
    const BSP::lightmap_t* lightmaps = map->lightmaps();
    const int numLightmaps = map->numLightmaps();
    for (int i = 0; i < numLightmaps; i ++) {
      const BSP::lightmap_t* lightmap = lightmaps + i;
      optional<GLuint> textureId = GLHelpers::loadTexture(lightmap, 128, 128, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
      if (textureId) {
        _lightmapTextures[i] = *textureId;
      } else {
        cerr << "failed to load lightmap " << i << "\n";
        return false;
      }
    }

    unsigned char white[] = { 255, 255, 255, 255 };
    optional<GLuint> textureId = GLHelpers::loadTexture(
        &white[0], 1, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    if (textureId) {
      _fallbackLightmapTexture = *textureId;
    } else {
      cerr << "failed to create fallback lightmap\n";
      return false;
    }
  }

  {
    const int numFaces = map->numFaces();
    for (int faceIndex = 0; faceIndex < numFaces; faceIndex ++) {
      optional<RenderableFace> face = RenderableFace::generate(map, faceIndex);
      if (face) {
        _renderableFaces.push_back(*face);
      }
    }
  }

  return true;
}

struct TesselatedPatch {
  vector<BSP::vertex_t> vertices;
  vector<int> indices;
};

static TesselatedPatch tesselatedPatch(int L, const vector<BSP::vertex_t>& controls) {
  // The body of this method is borrowed from: http://graphics.cs.brown.edu/games/quake/quake3.html#RenderingFaces
  // Thank you, Morgan McGuire!

  TesselatedPatch result;

  // The number of vertices along a side is 1 + num edges
  const int L1 = L + 1;
  result.vertices.resize(L1 * L1);

  // 0 1 2
  // 3 4 5
  // 6 7 8

  for (int col = 0; col <= L; ++col) { // Fill in the first row
    double a = (double)col / L;
    double b = 1 - a;

    result.vertices[col] =
        controls[0] * (b * b) + 
        controls[3] * (2 * b * a) +
        controls[6] * (a * a);
  }

  for (int row = 1; row <= L; ++row) { // Fill in each next row
    double a = (double)row / L;
    double b = 1.0 - a;

    BSP::vertex_t temp[3];
    const auto helper = [&controls, a, b](int controlRow) {
      return
        controls[controlRow * 3 + 0] * (b * b) + 
        controls[controlRow * 3 + 1] * (2 * b * a) +
        controls[controlRow * 3 + 2] * (a * a);
    };
    temp[0] = helper(0);
    temp[1] = helper(1);
    temp[2] = helper(2);

    for(int col = 0; col <= L; ++col) {
      double a = (double)col / L;
      double b = 1.0 - a;

      result.vertices[row * L1 + col]=
          temp[0] * (b * b) + 
          temp[1] * (2 * b * a) +
          temp[2] * (a * a);
    }
  }

  // Compute the indices
  for (int row = 0; row < L; ++row) {
    for(int col = 0; col < L; ++col)	{
      result.indices.push_back((row + 0) * L1 + (col + 0));
      result.indices.push_back((row + 1) * L1 + (col + 0));
      result.indices.push_back((row + 1) * L1 + (col + 1));

      result.indices.push_back((row + 0) * L1 + (col + 0));
      result.indices.push_back((row + 1) * L1 + (col + 1));
      result.indices.push_back((row + 0) * L1 + (col + 1));
    }
  }

  return result;
}

static TesselatedPatch untesselatedPatch(const vector<BSP::vertex_t>& controlPoints) {
  // 0 1 2
  // 3 4 5
  // 6 7 8

  assert(controlPoints.size() == 9);

  TesselatedPatch untesselated;
  untesselated.vertices.insert(untesselated.vertices.end(), begin(controlPoints), end(controlPoints));

  untesselated.indices.push_back(0);
  untesselated.indices.push_back(1);
  untesselated.indices.push_back(3);

  untesselated.indices.push_back(1);
  untesselated.indices.push_back(3);
  untesselated.indices.push_back(4);

  untesselated.indices.push_back(1);
  untesselated.indices.push_back(2);
  untesselated.indices.push_back(4);

  untesselated.indices.push_back(2);
  untesselated.indices.push_back(4);
  untesselated.indices.push_back(5);

  untesselated.indices.push_back(3 + 0);
  untesselated.indices.push_back(3 + 1);
  untesselated.indices.push_back(3 + 3);

  untesselated.indices.push_back(3 + 1);
  untesselated.indices.push_back(3 + 3);
  untesselated.indices.push_back(3 + 4);

  untesselated.indices.push_back(3 + 1);
  untesselated.indices.push_back(3 + 2);
  untesselated.indices.push_back(3 + 4);

  untesselated.indices.push_back(3 + 2);
  untesselated.indices.push_back(3 + 4);
  untesselated.indices.push_back(3 + 5);

  return untesselated;
}

static TesselatedPatch tesselatedFace(const BSPMap* map, const BSP::face_t* face) {
  assert(face->type == (int) BSP::FaceType::PATCH);

  const BSP::vertex_t* faceVertices = map->vertices() + face->vertex;

  int numVerticesWidth = face->size[0];
  int numVerticesHeight = face->size[1];

  assert(numVerticesWidth * numVerticesHeight == face->n_vertices);

  int numRows = (numVerticesHeight - 1) / 2;
  int numCols  = (numVerticesWidth - 1) / 2;

  int tesselationLevel = 7;

  vector<TesselatedPatch> allPatches;

  for (int row = 0; row < numRows; row ++) {
    for (int col = 0; col < numCols; col ++) {
      const BSP::vertex_t* patchVertices = faceVertices + row * 2 * numVerticesWidth + col * 2;

      vector<BSP::vertex_t> controlPoints; // this seems readable to me!
      controlPoints.resize(9);
      controlPoints[0] = patchVertices[0 * numVerticesWidth + 0];
      controlPoints[1] = patchVertices[0 * numVerticesWidth + 1];
      controlPoints[2] = patchVertices[0 * numVerticesWidth + 2];
      controlPoints[3] = patchVertices[1 * numVerticesWidth + 0];
      controlPoints[4] = patchVertices[1 * numVerticesWidth + 1];
      controlPoints[5] = patchVertices[1 * numVerticesWidth + 2];
      controlPoints[6] = patchVertices[2 * numVerticesWidth + 0];
      controlPoints[7] = patchVertices[2 * numVerticesWidth + 1];
      controlPoints[8] = patchVertices[2 * numVerticesWidth + 2];

      // allPatches.push_back(untesselatedPatch(controlPoints));
      allPatches.push_back(tesselatedPatch(tesselationLevel, controlPoints));
    }
  }

  TesselatedPatch result;

  for (const TesselatedPatch& patch : allPatches) {
    int startingVertexIndex = result.vertices.size();
    result.vertices.insert(result.vertices.end(), patch.vertices.begin(), patch.vertices.end());
    for (int index : patch.indices) {
      result.indices.push_back(startingVertexIndex + index);
    }
  }

  return result;
}

optional<RenderableFace> RenderableFace::generate(const BSPMap* map, int faceIndex) {
  const BSP::face_t* faces = map->faces();
  const BSP::face_t* face = faces + faceIndex;

  if (face->type == (int) BSP::FaceType::POLYGON || face->type == (int) BSP::FaceType::MESH) {
    const BSP::vertex_t* vertices = map->vertices();
    const BSP::meshvert_t* meshverts = map->meshverts();

    RenderableFace result;
    result.faceIndex = faceIndex;

    VBO& verticesVBO = result.vertices;
    VBO& colorsVBO = result.colors;
    EBO& elementsEBO = result.elements;

    // Load vertices
    glGenBuffers(1, &(verticesVBO.buffer));
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO.buffer);
    glBufferData(
      GL_ARRAY_BUFFER,
      sizeof(BSP::vertex_t) * face->n_vertices,
      &((vertices + face->vertex)->position),
      GL_STATIC_DRAW);

    verticesVBO.stride = sizeof(BSP::vertex_t);

    // Load colors
    colorsVBO = GLHelpers::generateRandomColorsVBO(face->n_vertices);

    // Load EBO
    glGenBuffers(1, &(elementsEBO.buffer));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO.buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
      sizeof(GLuint) * face->n_meshverts,
      meshverts + face->meshvert,
      GL_STATIC_DRAW);
    elementsEBO.count = face->n_meshverts;

    return result;
  }

  if (face->type == (int) BSP::FaceType::PATCH) {
    RenderableFace result;
    result.faceIndex = faceIndex;

    VBO& verticesVBO = result.vertices;
    VBO& colorsVBO = result.colors;
    EBO& elementsEBO = result.elements;

    TesselatedPatch tesselation = tesselatedFace(map, face);

    // cout << "num vertices " << tesselation.vertices.size() << " num elements " << tesselation.indices.size() << "\n";

    // Load vertices
    glGenBuffers(1, &(verticesVBO.buffer));
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO.buffer);
    glBufferData(
      GL_ARRAY_BUFFER,
      sizeof(BSP::vertex_t) * tesselation.vertices.size(),
      &(tesselation.vertices.data()->position),
      GL_STATIC_DRAW);

    verticesVBO.stride = sizeof(BSP::vertex_t);

    // Load colors
    colorsVBO = GLHelpers::generateRandomColorsVBO(tesselation.vertices.size());

    // Load EBO
    glGenBuffers(1, &(elementsEBO.buffer));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO.buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
      sizeof(GLuint) * tesselation.indices.size(),
      tesselation.indices.data(),
      GL_STATIC_DRAW);
    elementsEBO.count = tesselation.indices.size();

    return result;
  }

  return {};
}

void RenderableBSP::render(const SceneShaderParameters& inputs, RenderMode mode, const optional<HitScanResult>& result) {
  const BSPMap* map = _map.get();
  if (!map) {
    cerr << "map failed to load\n";
    return;
  }

  const BSP::face_t* faces = map->faces();
  const int numFaces = map->numFaces();

  const BSP::texture_t* textures = map->textures();
  const int numTextures = map->numTextures();

  for (const RenderableFace& renderableFace : _renderableFaces) {
    const BSP::face_t* face = faces + renderableFace.faceIndex;
    // if (face->type != (int) BSP::FaceType::PATCH) {
    //   continue;
    // }

    const VBO& verticesVBO = renderableFace.vertices;
    const VBO& colorsVBO = renderableFace.colors;
    const EBO& elementsEBO = renderableFace.elements;

    // Bind the texture
    int textureOffset = face->texture;
    if (textureOffset < 0 || textureOffset >= numTextures) {
      continue;
    }

    const BSP::texture_t* texture = textures + textureOffset;
    const int textureResourceId = _textureResourceIds[string(texture->name)];

    optional<RenderableTextureOptions> textureOptions = ResourceManager::getInstance()->getTextureOptions(textureResourceId);
    bool isTransparent = textureOptions ? textureOptions->surfaceParamTrans : false;
    if (mode == RenderMode::SOLID && isTransparent) {
      continue;
    } else if (mode == RenderMode::TRANSPARENCY && !isTransparent) {
      continue;
    }
    glUniform1f(inputs.unifAlpha, isTransparent ? 0.9 : 1);

    optional<GLuint> textureId = ResourceManager::getInstance()->getTexture(textureResourceId);
    if (textureId) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, *textureId);
      glUniform1i(inputs.unifTexture, 0);
    }

    if (result && result->face == face) {
      glUniform1i(inputs.unifHighlight, 1);

      static int printLimiter = 0;
      if (result && printLimiter ++ % 100 == 0) {
        cout << "current face: " << *face << ", " << *texture << "\n";
      }
    } else {
      glUniform1i(inputs.unifHighlight, 0);
    }

    // Bind the lightmap
    int lightmapIndex = face->lm_index;
    if (const GLuint* textureId = getValue(_lightmapTextures, lightmapIndex)) {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, *textureId);
      glUniform1i(inputs.unifLightmapTexture, 1);
    } else {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, _fallbackLightmapTexture);
      glUniform1i(inputs.unifLightmapTexture, 1);
    }

    // Bind vertices
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO.buffer);
    glVertexAttribPointer(
      inputs.inPosition, 3, GL_FLOAT, GL_FALSE,
      verticesVBO.stride /* stride */,
      (void*) 0 /* offset */);

    // Bind texture coordinates
    glVertexAttribPointer(
      inputs.inTextureCoords, 2, GL_FLOAT, GL_FALSE,
      verticesVBO.stride /* stride */,
      (void*) offsetof(BSP::vertex_t, texcoord) /* offset */);

    // Bind lightmap coordinates
    glVertexAttribPointer(
      inputs.inLightmapCoords, 2, GL_FLOAT, GL_FALSE,
      verticesVBO.stride /* stride */,
      (void*) offsetof(BSP::vertex_t, lmcoord) /* offset */);

    // Bind colors
    glBindBuffer(GL_ARRAY_BUFFER, colorsVBO.buffer);
    glVertexAttribPointer(
      inputs.inColor, 3, GL_FLOAT, GL_FALSE,
      colorsVBO.stride /* stride */,
      (void*) 0 /* offset */);

    // Render elements
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsEBO.buffer);
    glDrawElements(GL_TRIANGLES, elementsEBO.count, GL_UNSIGNED_INT, 0);
  }
}