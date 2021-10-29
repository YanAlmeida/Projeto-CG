#include "clouds.hpp"

#include <cppitertools/itertools.hpp>

void Clouds::initializeGL(GLuint program, int quantity) {
  terminateGL();

  m_program = program;
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  // Cria nuvens
  m_clouds.clear();
  m_clouds.resize(quantity);
  float dist = 0;
  float dist_c_to_e = 2 * m_radius * m_scale + 0.05;
  float dist_side = 0.05f;
  for (auto &cloud : m_clouds) {
    cloud = generateCloud(glm::vec2{-1 + dist_c_to_e + dist_side + dist,
                                    1 - (m_radius * m_scale + 0.01)});
    dist += 2 * (1 - (dist_c_to_e + dist_side)) / (quantity - 1);
  }
}

void Clouds::paintGL() {
  abcg::glUseProgram(m_program);

  for (const auto &cloud : m_clouds) {
    float dist = m_radius * m_scale + 0.05f;
    for (const int mod : {-1, 0, 1}) {
      abcg::glBindVertexArray(cloud.m_vao);

      abcg::glUniform4fv(m_colorLoc, 1, &cloud.m_color.r);
      abcg::glUniform1f(m_scaleLoc, m_scale);

      abcg::glUniform2f(m_translationLoc, cloud.m_translation.x + mod * dist,
                        cloud.m_translation.y);

      abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, cloud.m_polygonSides + 2);

      abcg::glBindVertexArray(0);
    }
  }

  abcg::glUseProgram(0);
}

void Clouds::terminateGL() {
  for (auto cloud : m_clouds) {
    abcg::glDeleteBuffers(1, &cloud.m_vbo);
    abcg::glDeleteVertexArrays(1, &cloud.m_vao);
  }
}

// Função para gerar nuvens de acordo com posição dada (posição do circulo
// central)
Clouds::Cloud Clouds::generateCloud(glm::vec2 translation) {
  Cloud cloud;
  cloud.m_translation = translation;
  cloud.m_color = m_cloud_color;

  // Create geometry
  std::vector<glm::vec2> positions(0);
  positions.emplace_back(0, 0);
  const auto step{M_PI * 2 / cloud.m_polygonSides};
  for (const auto angle : iter::range(0.0, M_PI * 2, step)) {
    positions.emplace_back(m_radius * std::cos(angle),
                           m_radius * std::sin(angle));
  }
  positions.push_back(positions.at(1));

  // Generate VBO
  abcg::glGenBuffers(1, &cloud.m_vbo);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, cloud.m_vbo);
  abcg::glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                     positions.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  abcg::glGenVertexArrays(1, &cloud.m_vao);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(cloud.m_vao);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, cloud.m_vbo);
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);

  return cloud;
}