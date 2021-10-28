#ifndef STARLAYERS_HPP_
#define STARLAYERS_HPP_

#include <list>
#include <random>

#include "abcg.hpp"

class OpenGLWindow;

class Clouds {
 public:
  void initializeGL(GLuint program, int quantity);
  void paintGL();
  void terminateGL();

 private:
  friend OpenGLWindow;

  GLuint m_program{};
  GLint m_colorLoc{};
  GLint m_translationLoc{};
  GLint m_scaleLoc{};
  float m_radius{0.5f};
  float m_scale{0.25};
  struct Cloud {
    GLuint m_vao{};
    GLuint m_vbo{};

    glm::vec4 m_color{1};
    int m_polygonSides{50};

    
    glm::vec2 m_translation{glm::vec2(0)};
  };

  std::list<Cloud> m_clouds;

  Clouds::Cloud generateCloud(glm::vec2 translation);

};

#endif