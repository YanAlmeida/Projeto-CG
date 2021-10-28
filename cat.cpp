#include "cat.hpp"

#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/rotate_vector.hpp>

void Cat::initializeGL(GLuint program) {
  terminateGL();

  m_program = program;
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  m_rotation = 0.0f;
  m_translation = glm::vec2(0);
  m_velocity = glm::vec2(0);

  // clang-format off
  std::array<glm::vec2, 32> positions{
      // Cat body
      glm::vec2{-15.00f, +15.00f}, glm::vec2{-10.00f, +10.00f},
      glm::vec2{-15.00f, +10.00f}, glm::vec2{00.0f, +15.5f},
      glm::vec2{-05.00f, +10.00f}, glm::vec2{00.00f, +10.00f},
      glm::vec2{-15.00f, 00.00f}, glm::vec2{+00.0f, +00.00f},
      glm::vec2{-10.00f, 00.00f}, glm::vec2{-08.00f, 0.00f},
      glm::vec2{-10.00f, -15.00f}, glm::vec2{-08.00f, -15.00f},
      glm::vec2{-08.00f, -09.00f}, glm::vec2{08.00f, -09.00f},
      glm::vec2{+08.00f, 00.00f}, glm::vec2{+10.00f, 00.00f},
      glm::vec2{+10.00f, -15.00f}, glm::vec2{+08.00f, -15.00f},
      glm::vec2{+10.00f, -02.0f}, glm::vec2{+15.50f, -02.00f},
      glm::vec2{+15.50f, 00.00f}, glm::vec2{+13.50f, 00.00f},
      glm::vec2{+13.50f, 06.00f}, glm::vec2{+15.50f, 06.00f},
      };

  // Normalize
  for (auto &position : positions) {
    position /= glm::vec2{15.5f, 15.5f};
  }

  const std::array indices{0, 1, 2,
                           3, 4, 5,
                           2, 5, 7,
                           2, 6, 7,
                           8, 10, 11,
                           8, 9, 11,
                           9, 12, 13,
                           9, 14, 13,
                           15, 16, 17,
                           14, 15, 17, 
                           18, 19, 20,
                           15, 18, 20,
                           20, 21, 22,
                           20, 22, 23
                          };
  // clang-format on

  // Generate VBO
  abcg::glGenBuffers(1, &m_vbo);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  abcg::glGenBuffers(1, &m_ebo);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  abcg::glGenVertexArrays(1, &m_vao);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_vao);

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);
}

void Cat::paintGL(const GameData &gameData){
  if (gameData.m_state != State::Playing) return;

  abcg::glUseProgram(m_program);

  abcg::glBindVertexArray(m_vao);

  abcg::glUniform1f(m_scaleLoc, m_scale);
  abcg::glUniform1f(m_rotationLoc, m_rotation);
  abcg::glUniform2fv(m_translationLoc, 1, &m_translation.x);

  abcg::glUniform4fv(m_colorLoc, 1, &m_color.r);
  abcg::glDrawElements(GL_TRIANGLES, 14 * 3, GL_UNSIGNED_INT, nullptr);

  abcg::glBindVertexArray(0);

  abcg::glUseProgram(0);
}

void Cat::terminateGL() {
  abcg::glDeleteBuffers(1, &m_vbo);
  abcg::glDeleteBuffers(1, &m_ebo);
  abcg::glDeleteVertexArrays(1, &m_vao);
}


void Cat::update(const GameData &gameData, float deltatime) {
  // Move
  if (gameData.m_input[static_cast<size_t>(Input::Left)] && m_translation.x>-(1-m_scale))
    m_translation += (glm::vec2{-0.7f, 0.0f})*deltatime;
  if (gameData.m_input[static_cast<size_t>(Input::Right)] && m_translation.x<(1-m_scale))
    m_translation += (glm::vec2{0.7f, 0.0f})*deltatime;
  if (gameData.m_input[static_cast<size_t>(Input::Up)] && m_translation.y<(1-m_scale))
    m_translation += (glm::vec2{0.0f, 0.7f})*deltatime;
  if (gameData.m_input[static_cast<size_t>(Input::Down)] && m_translation.y>-(1-m_scale))
    m_translation += (glm::vec2{0.0f, -0.7f})*deltatime;
}
