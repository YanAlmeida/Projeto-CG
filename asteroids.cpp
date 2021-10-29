#include "asteroids.hpp"

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

void Asteroids::initializeGL(GLuint program, int quantity) {
  terminateGL();

  //Inicia um gerador de numeros pseudo-aleatórios
  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  m_program = program;
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  // Cria asteroids
  m_asteroids.clear();
  m_asteroids.resize(quantity);

  for (auto &asteroid : m_asteroids) {
    asteroid = createAsteroid(glm::vec2{m_randomDist(m_randomEngine), 1});
  }
}

void Asteroids::paintGL() {
  abcg::glUseProgram(m_program);

  for (const auto &asteroid : m_asteroids) {
    abcg::glBindVertexArray(asteroid.m_vao);

    abcg::glUniform4fv(m_colorLoc, 1, &asteroid.m_color.r);
    abcg::glUniform1f(m_scaleLoc, asteroid.m_scale);
    abcg::glUniform1f(m_rotationLoc, asteroid.m_rotation);

    abcg::glUniform2f(m_translationLoc, asteroid.m_translation.x,
                      asteroid.m_translation.y);

    abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, asteroid.m_polygonSides + 2);

    abcg::glBindVertexArray(0);
  }

  abcg::glUseProgram(0);
}

void Asteroids::terminateGL() {
  for (auto asteroid : m_asteroids) {
    abcg::glDeleteBuffers(1, &asteroid.m_vbo);
    abcg::glDeleteVertexArrays(1, &asteroid.m_vao);
  }
}

// Atualizacao dos asteroids (girar e se mover na tela)
void Asteroids::update(float deltaTime, int *m_pedras_desviados_pointer) {
  for (auto &asteroid : m_asteroids) {
    asteroid.m_rotation = glm::wrapAngle(
        asteroid.m_rotation + asteroid.m_angularVelocity * deltaTime);
    asteroid.m_translation += asteroid.m_velocity * deltaTime;

    if (asteroid.m_translation.y < -(1.0f + asteroid.m_scale) ||
        asteroid.m_translation.y > (1.0f + asteroid.m_scale)) {
      asteroid.m_hit = 1;
      *(m_pedras_desviados_pointer) += 1;
    }
  }
}

// Criando asteroids com posição, constante de velocidade inversa, ordenacao
// (cima ou baixo) e escala
Asteroids::Asteroid Asteroids::createAsteroid(glm::vec2 translation,
                                              float inverse_velocity,
                                              int ordenation, float scale) {
  Asteroid asteroid;

  auto &re{m_randomEngine};  

  // Escolher numero de lados aleatoriamente
  std::uniform_int_distribution<int> randomSides(8, 10);
  asteroid.m_polygonSides = randomSides(re);

  // Escolher uma cor aleatoria na escala
  std::uniform_real_distribution<float> randomIntensity{0.6f, 0.9f};
  asteroid.m_color = m_color_asteroids * randomIntensity(re);

  asteroid.m_rotation = 0.0f;
  asteroid.m_scale = scale;
  asteroid.m_translation = translation;

  // Velocidade angular aleatoria
  asteroid.m_angularVelocity = m_randomDist(re);

  // Direção aleatoria
  glm::vec2 direction{0, -1};

  if (ordenation) {
    direction.y = 1;
  }

  asteroid.m_velocity = glm::normalize(direction) / inverse_velocity;

  // Criar geometria
  std::vector<glm::vec2> positions(0);
  positions.emplace_back(0, 0);
  const auto step{M_PI * 2 / asteroid.m_polygonSides};
  std::uniform_real_distribution<float> randomRadius(0.6f, 0.8f);
  for (const auto angle : iter::range(0.0, M_PI * 2, step)) {
    const auto radius{randomRadius(re)};
    positions.emplace_back(radius * std::cos(angle), radius * std::sin(angle));
  }
  positions.push_back(positions.at(1));

  // Criar VBO
  abcg::glGenBuffers(1, &asteroid.m_vbo);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, asteroid.m_vbo);
  abcg::glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                     positions.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Pegar localizacao dos atributos
  GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};

  // Criar VAO
  abcg::glGenVertexArrays(1, &asteroid.m_vao);

  // Vincular atributos de vértice ao VAO atual
  abcg::glBindVertexArray(asteroid.m_vao);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, asteroid.m_vbo);
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  //  Fim da ligação ao VAO atual
  abcg::glBindVertexArray(0);

  return asteroid;
}
