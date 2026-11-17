////////////////////////////////////////////////////////////////////////////////
//
// Draws the seven Tangram pieces directly in clip space.
//
// Copyright (c) 2013-26 by Carlos Martinho
//
////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <cstddef>
#include <memory>

#include "../mgl/mgl.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////////////////// MYAPP

struct Vertex;

class MyApp : public mgl::App {
public:
  MyApp() = default;
  ~MyApp() override = default;

  void initCallback(GLFWwindow *win) override;
  void windowSizeCallback(GLFWwindow* win, int width, int height) override;
  void displayCallback(GLFWwindow *win, double elapsed) override;
  void windowCloseCallback(GLFWwindow *win) override;

private:
  const GLuint POSITION = 0, COLOR = 1;
  struct Shape {
    GLuint VaoId;
    GLuint Buffers[2];
    GLsizei IndexCount;
  };
  enum Mesh { TriangleMesh, SquareMesh, ParallelogramMesh, MeshCount };
  class Piece {
  public:
    Piece(Mesh mesh, glm::vec2 position, float angle, float scale,
          glm::vec4 color, bool flipX = false)
        : mesh(mesh), position(position), angle(angle), scale(scale),
          color(color), flipX(flipX) {}

    void draw(const Shape (&shapes)[MeshCount], GLint matrixId,
              GLuint colorAttribute) const;

  private:
    Mesh mesh;
    glm::vec2 position;
    float angle;
    float scale;
    glm::vec4 color;
    bool flipX;
  };

  class Layout {
  public:
    Piece pieces[7];

    void draw(const Shape (&shapes)[MeshCount], GLint matrixId,
              GLuint colorAttribute) const;
  };

  Shape Shapes[MeshCount];
  std::unique_ptr<mgl::ShaderProgram> Shaders = nullptr;
  GLint MatrixId;

  void createShaderProgram();
  template <std::size_t VertexCount, std::size_t IndexCount>
  Shape createShape(const Vertex (&vertices)[VertexCount],
                    const GLubyte (&indices)[IndexCount]);
  void createBufferObjects();
  void destroyBufferObjects();
  void drawScene();
};

//////////////////////////////////////////////////////////////////////// SHADERs

void MyApp::createShaderProgram() {
  Shaders = std::make_unique<mgl::ShaderProgram>();
  Shaders->addShader(GL_VERTEX_SHADER, "src/clip-vs.glsl");
  Shaders->addShader(GL_FRAGMENT_SHADER, "src/clip-fs.glsl");

  Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, POSITION);
  Shaders->addAttribute(mgl::COLOR_ATTRIBUTE, COLOR);
  Shaders->addUniform("Matrix");

  Shaders->create();

  MatrixId = Shaders->Uniforms["Matrix"].index;
}

//////////////////////////////////////////////////////////////////// VAOs & VBOs

struct Vertex {
  GLfloat XYZW[4];
};

const float Root2 = std::sqrtf(2.0f);
const float SmallLeg = Root2 / 4.0f;

// small triangle
const Vertex Triangle[] = {
    {{0.0f, 0.0f, 0.0f, 1.0f}},
    {{SmallLeg, 0.0f, 0.0f, 1.0f}},
    {{0.0f, SmallLeg, 0.0f, 1.0f}},
};

const Vertex Square[] = {
    {{ 0.25f,  0.0f,  0.0f, 1.0f}},
    {{ 0.0f,   0.25f, 0.0f, 1.0f}},
    {{-0.25f,  0.0f,  0.0f, 1.0f}},
    {{ 0.0f,  -0.25f, 0.0f, 1.0f}},
};

const Vertex Parallelogram[] = {
    {{-0.375f, -0.125f, 0.0f, 1.0f}},
    {{ 0.125f, -0.125f, 0.0f, 1.0f}},
    {{ 0.375f,  0.125f, 0.0f, 1.0f}},
    {{-0.125f,  0.125f, 0.0f, 1.0f}},
};

const GLubyte TriangleIndices[] = {0, 1, 2};
const GLubyte QuadIndices[] = {0, 1, 2, 0, 2, 3};

template <std::size_t VertexCount, std::size_t IndexCount>
MyApp::Shape MyApp::createShape(const Vertex (&vertices)[VertexCount],
                               const GLubyte (&indices)[IndexCount]) {
  Shape shape{};
  shape.IndexCount = IndexCount;
  glGenVertexArrays(1, &shape.VaoId);
  glBindVertexArray(shape.VaoId);
  glGenBuffers(2, shape.Buffers);

  glBindBuffer(GL_ARRAY_BUFFER, shape.Buffers[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(this->POSITION);
  glVertexAttribPointer(this->POSITION, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.Buffers[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return shape;
}

void MyApp::createBufferObjects() {
  Shapes[TriangleMesh] = createShape(Triangle, TriangleIndices);
  Shapes[SquareMesh] = createShape(Square, QuadIndices);
  Shapes[ParallelogramMesh] = createShape(Parallelogram, QuadIndices);
}

void MyApp::destroyBufferObjects() {
  for (const Shape &shape : Shapes) {
    glDeleteVertexArrays(1, &shape.VaoId);
    glDeleteBuffers(2, shape.Buffers);
  }
}

////////////////////////////////////////////////////////////////////////// SCENE

void MyApp::Piece::draw(const Shape (&shapes)[MeshCount], GLint matrixId,
                        GLuint colorAttribute) const {
  const Shape &shape = shapes[this->mesh];
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(this->position, 0.0f));
  model = glm::rotate(model, glm::radians(this->angle), glm::vec3(0, 0, 1));
  const float scaleX = this->flipX ? -this->scale : this->scale;
  model = glm::scale(model, glm::vec3(scaleX, this->scale, 1.0f));
  glUniformMatrix4fv(matrixId, 1, GL_FALSE, glm::value_ptr(model));

  glBindVertexArray(shape.VaoId);
  // With no color array enabled, the color attribute is constant for this draw.
  glVertexAttrib4fv(colorAttribute, glm::value_ptr(this->color));
  glFrontFace(this->flipX ? GL_CW : GL_CCW);
  glDrawElements(GL_TRIANGLES, shape.IndexCount, GL_UNSIGNED_BYTE, nullptr);
}

void MyApp::Layout::draw(const Shape (&shapes)[MeshCount], GLint matrixId,
                         GLuint colorAttribute) const {
  for (const Piece &piece : this->pieces) {
    piece.draw(shapes, matrixId, colorAttribute);
  }
  glFrontFace(GL_CCW);
  glBindVertexArray(0);
}

void MyApp::drawScene() {
  // original tangram board placement
  /*
  static const Layout squareLayout = {{
      {TriangleMesh, {0.0f, 0.0f}, 135.0f, 2.0f, {1.0f, 0.0f, 1.0f, 1.0f}}, // large magenta triangle
      {TriangleMesh, {0.0f, 0.0f}, 45.0f, 2.0f, {0.0f, 0.0f, 1.0f, 1.0f}}, // large blue triangle
      {TriangleMesh, {0.25f, 0.25f}, -45.0f, 1.0f, {1.0f, 0.0f, 0.0f, 1.0f}}, // small red triangle
      {TriangleMesh, {0.0f, 0.0f}, -135.0f, 1.0f, {0.0f, 1.0f, 1.0f, 1.0f}}, // small blue triangle
      {SquareMesh, {0.25f, 0.0f}, 0.0f, 1.0f, {0.0f, 1.0f, 0.0f, 1.0f}},
      {TriangleMesh, {0.5f, -0.5f}, 90.0f, Root2, {0.5f, 0.0f, 0.5f, 1.0f}}, // medium (purple) triangle
      {ParallelogramMesh, {-0.125f, -0.375f}, 0.0f, 1.0f, {1.0f, 0.5f, 0.0f, 1.0f}},
  }};
  */

  static const Layout snailLayout = {{
      {TriangleMesh, {0.25f, 0.5f}, -135.0f, 2.0f, {1.0f, 0.0f, 1.0f, 1.0f}}, // large magenta triangle
      {TriangleMesh, {0.25f, -0.5f}, 45.0f, 2.0f, {0.0f, 0.0f, 1.0f, 1.0f}}, // large blue triangle
      {TriangleMesh, {0.25f + SmallLeg, -0.5f + SmallLeg}, -135.0f, Root2, {0.5f, 0.0f, 0.5f, 1.0f}}, // medium (purple) triangle
      {ParallelogramMesh, {0.25f - 0.375f, -0.5f + 0.125f}, 0.0f, 1.0f, {1.0f, 0.5f, 0.0f, 1.0f}, true}, 
      {TriangleMesh, {-0.5f, -0.5f + 0.25f}, -135.0f, 1.0f, {0.0f, 1.0f, 1.0f, 1.0f}}, // small blue triangle
      {TriangleMesh, {-0.5f, -0.25f}, 0.0f, 1.0f, {1.0f, 0.0f, 0.0f, 1.0f}}, // small red triangle
      {SquareMesh, {-0.5f - SmallLeg / 2.0f, -0.25f + SmallLeg / 2.0f}, 45.0f, 1.0f, {0.0f, 1.0f, 0.0f, 1.0f}},
  }};

  Shaders->bind();
  snailLayout.draw(Shapes, MatrixId, COLOR);
  Shaders->unbind();
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow *win [[maybe_unused]]) {
  createBufferObjects();
  createShaderProgram();
}

void MyApp::windowSizeCallback(GLFWwindow *win [[maybe_unused]], int width, int height) {
    drawScene();
}

void MyApp::windowCloseCallback(GLFWwindow *win [[maybe_unused]]) {
  destroyBufferObjects();
}

void MyApp::displayCallback(GLFWwindow *win [[maybe_unused]],
                            double elapsed [[maybe_unused]]) {
  drawScene();
}

/////////////////////////////////////////////////////////////////////////// MAIN

int main() {
  mgl::Engine &engine = mgl::Engine::getInstance();
  engine.setApp(std::make_unique<MyApp>());
#ifdef __APPLE__
  engine.setOpenGL(4, 1);
#else
  engine.setOpenGL(4, 6);
#endif
  engine.setWindow(600, 600, "A2 - Tangram 2D", 0, 1);
  engine.init();
  engine.run();
  return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////// END
