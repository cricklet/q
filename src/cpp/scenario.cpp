#include "scenario.h"

#include "resources.h"
#include "resource_manager.h"
#include "renderable.h"
#include "gl_helpers.h"
#include "pprint.hpp"

TextureRenderer::TextureRenderer(TextureRendererMode mode) {
  if (mode == TextureRendererMode::FLIP_VERTICALLY) {
    for (int i = 0; i < sizeof(_vertices) / sizeof(_vertices[0]); i += 4) {
      _vertices[i + 1] = - _vertices[i + 1];
    }
  }

  cout << "starting to load TextureRenderer\n";
  _shaderResourceID = ResourceManager::nextID();
  ResourceManager::getInstance()->loadShaders(this, {
    "./src/glsl/test.vert",
    "./src/glsl/test.frag",
    _shaderResourceID
  });
}

bool TextureRenderer::finishLoading() {
  cout << "TextureRenderer::finishLoading\n";
  
  // Create a VAO for the attribute configuration
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  ////////////////////////////////////////////////////////////////////////////
  // Generate EBO
  glGenBuffers(1, &_ebo);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_elements), _elements, GL_STATIC_DRAW);
  
  if (hasErrors()) {
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////
  // Generate VBO
  glGenBuffers(1, &_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);

  // Copy the vertex data into the vbo
  glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);
  
  if (hasErrors()) {
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////
  // Get the test shader program
  optional<GLuint> shaderProgram = ResourceManager::getInstance()->getShaderProgram(_shaderResourceID);
  if (!shaderProgram) {
    cerr << "failed to load shader program in TextureRenderer::load\n";
    return false;
  }

  // Use the program...
  glLinkProgram(*shaderProgram);
  glUseProgram(*shaderProgram);
  
  if (hasErrors()) {
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////
  // Specify the inputs

  _inPosition = glGetAttribLocation(*shaderProgram, "inPosition");
  glEnableVertexAttribArray(_inPosition);

  _inTextureCoords = glGetAttribLocation(*shaderProgram, "inTextureCoords");
  glEnableVertexAttribArray(_inTextureCoords);

  _unifTexture = glGetUniformLocation(*shaderProgram, "unifTexture");

  if (hasErrors()) {
    return false;
  }

  return true;
}

void TextureRenderer::render(vector<GLuint> textureIDs) {
  optional<GLuint> shaderProgram = ResourceManager::getInstance()->getShaderProgram(_shaderResourceID);
  if (!shaderProgram) {
    cerr << "failed to load shader program in TextureRenderer::render\n";
    return;
  }

  glUseProgram(*shaderProgram);
  glBindVertexArray(_vao);

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);

  for (const auto textureID : textureIDs) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(_unifTexture, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glVertexAttribPointer(_inPosition, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(_inTextureCoords, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));

    glDrawElements(GL_TRIANGLES, sizeof(_elements) / sizeof(_elements[0]), GL_UNSIGNED_INT, 0);
  }

  hasErrors();
}

TestScenario::TestScenario() {
}

static const char* testVertexSource =
R"glsl(#version 300 es
in vec2 position;
void main() {
  gl_Position = vec4(position, 0.0, 1.0);
})glsl";
static const char* testFragmentSource =
R"glsl(#version 300 es
out lowp vec4 outColor;
void main() {
  outColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)glsl";

bool TestScenario::finishLoading() {// Shader sources
  static float vertices[] = {
    0.0f,  0.5f, // Vertex 1 (X, Y)
    0.5f, -0.5f, // Vertex 2 (X, Y)
    -0.5f, -0.5f  // Vertex 3 (X, Y)
  };

  // Create a VAO for the attribute configuration
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  ////////////////////////////////////////////////////////////////////////////
  // Generate VBO
  GLuint vbo;
  glGenBuffers(1, &vbo);

  // Choose the vbo...
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // Copy the vertex data into the vbo
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  hasErrors();

  optional<GLuint> shader = GLHelpers::compileShaderProgram(testVertexSource, string(testVertexSource).length(), testFragmentSource, string(testFragmentSource).length());
  if (!shader) {
    cerr << "failed to load shader\n";
    return false;
  }
  hasErrors();

  // Use the program...
  glUseProgram(*shader);
  hasErrors();

  // Specify the layout of the vertices
  GLint posAttrib = glGetAttribLocation(*shader, "position");
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(posAttrib);
  hasErrors();

  return true;
}

void TestScenario::render() {
  // Clear the screen to black
  glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Draw a triangle from the 3 vertices
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

PopTartScenario::PopTartScenario() {
  _textureResourceID = ResourceManager::nextID();
  ResourceManager::getInstance()->loadResource(this, {
    "./data/textures/poptart.jpg",
    ResourceType::IMAGE_FILE,
    _textureResourceID
  });

  cout << "starting PopTartScenario\n";
  _renderer = shared_ptr<TextureRenderer>(new TextureRenderer());
}

bool PopTartScenario::finishLoading() {
  return true;
}

void PopTartScenario::render() {
  optional<GLuint> textureId = ResourceManager::getInstance()->getTexture(_textureResourceID);
  if (textureId) {
    _renderer->render({*textureId});
  }
}
