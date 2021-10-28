#include "openglwindow.hpp"

#include <imgui.h>

#include "abcg.hpp"

#include <iostream>

#include <iomanip>

void OpenGLWindow::handleEvent(SDL_Event &event) {
  // Keyboard events
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
      m_gameData.m_input.set(static_cast<size_t>(Input::Up));
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_gameData.m_input.set(static_cast<size_t>(Input::Down));
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.set(static_cast<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.set(static_cast<size_t>(Input::Right));
  }
  if (event.type == SDL_KEYUP) {
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Up));
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Down));
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Right));
  }

  if (event.type == SDL_MOUSEMOTION) {
    glm::ivec2 mousePosition;
    SDL_GetMouseState(&mousePosition.x, &mousePosition.y);

    glm::vec2 position{
        glm::vec2{(float)mousePosition.x / (m_viewportWidth / 2) - 1,
                  (float)mousePosition.y / (m_viewportHeight / 2) - 1}};

    position.y = -position.y;

    if (position.x > -(1 - m_cat.m_scale) && position.x < (1 - m_cat.m_scale))
      m_cat.m_translation.x = position.x;
    if (position.y > -(1 - m_cat.m_scale) && position.y < (1 - m_cat.m_scale))
      m_cat.m_translation.y = position.y;
  }
}

void OpenGLWindow::initializeGL() {
  // Load a new font
  ImGuiIO &io{ImGui::GetIO()};
  const auto filename{getAssetsPath() + "Inconsolata-Medium.ttf"};
  m_font = io.Fonts->AddFontFromFileTTF(filename.c_str(), 40.0f);
  if (m_font == nullptr) {
    throw abcg::Exception{abcg::Exception::Runtime("Cannot load font file")};
  }

  // Create program to render the other objects
  m_objectsProgram = createProgramFromFile(getAssetsPath() + "objects.vert",
                                           getAssetsPath() + "objects.frag");

  abcg::glClearColor(0.2f, 0.5f, 0.9f, 1);

#if !defined(__EMSCRIPTEN__)
  abcg::glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  // Start pseudo-random number generator
  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  m_clouds.initializeGL(m_objectsProgram, 3);
  m_cat.initializeGL(m_objectsProgram);
  m_asteroids.initializeGL(m_objectsProgram, 1);
}

void OpenGLWindow::restart() {
  m_rounds = 0;
  m_pedras_desviadas = 0;
  m_controlTimer.restart();
  m_ScreenTimer.restart();
  m_gameTimer.restart();
  m_gameData.m_state = State::Playing;

  m_clouds.initializeGL(m_objectsProgram, 3);
  m_cat.initializeGL(m_objectsProgram);
  m_asteroids.initializeGL(m_objectsProgram, 1);
}

void OpenGLWindow::update() {
  const float deltaTime{static_cast<float>(getDeltaTime())};

  m_cat.update(m_gameData, deltaTime);
  m_asteroids.update(deltaTime, &m_pedras_desviadas);
  float interval;

  if (60 - m_ScreenTimer.elapsed() < 30 && 60 - m_ScreenTimer.elapsed() > 10) {
    interval = 0.75f;
  } else if (60 - m_ScreenTimer.elapsed() < 10) {
    interval = 1.2f;
  } else {
    interval = 1.0f;
  }

  if (m_controlTimer.elapsed() > 1.0f && m_gameData.m_state == State::Playing) {
    m_controlTimer.restart();
    m_rounds += 1;
  }

  if (m_gameTimer.elapsed() > interval) {
    m_gameTimer.restart();
    std::uniform_real_distribution<float> m_randomDist{-1.0f, 1.0f};
    std::generate_n(std::back_inserter(m_asteroids.m_asteroids), 1, [&]() {
      int ordenation = signbit(m_randomDist(m_randomEngine));
      int starting_point = 1;
      if (ordenation) starting_point = -1;
      float velocity = (float)(m_total_time - m_ScreenTimer.elapsed()) / 12.0f;
      return m_asteroids.createAsteroid(
          glm::vec2{m_randomDist(m_randomEngine), starting_point}, velocity,
          ordenation);
    });
  }

  if (m_gameData.m_state == State::Playing) {
    checkCollisions();
    checkWinCondition();
  }
}

void OpenGLWindow::paintGL() {
  update();

  abcg::glClear(GL_COLOR_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  m_clouds.paintGL();
  m_asteroids.paintGL();
  m_cat.paintGL(m_gameData);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();
  // texto que aparece durante o game
  if (m_gameData.m_state == State::Playing) {
// conversoes
#include <string>
    std::string s = std::to_string(m_pedras_desviadas);
    char const *pchar = s.c_str();

    double tempo_restante = m_total_time - m_ScreenTimer.elapsed();
    std::string s2 = std::to_string(tempo_restante);
    char const *pchar2 = s2.c_str();

    // definições do imgui
    const auto size{ImVec2(720, 170)};
    const auto position{ImVec2(0, 0)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);

    ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                           ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoInputs};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font);

    ImGui::Columns(2);
    ImGui::SetColumnWidth(1, 100);
    ImGui::Text("Pedras desviadas:");
    ImGui::Text("Tempo restante:");
    ImGui::NextColumn();
    ImGui::SetColumnWidth(1, 110);
    ImGui::Text(pchar);
    ImGui::Text(pchar2);

    ImGui::PopFont();
    ImGui::End();
  } else {
    const auto size{ImVec2(380, 200)};
    const auto position{ImVec2((m_viewportWidth - size.x) / 2.0f,
                               (m_viewportHeight - size.y) / 2.0f)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                           ImGuiWindowFlags_NoTitleBar};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font);

    if (m_gameData.m_state == State::Initial) {
      ImGui::Button("Iniciar", ImVec2(-1, 50));
      // See also IsItemHovered, IsItemActive, etc
      if (ImGui::IsItemClicked()) {
        restart();
      }
      static int enabled=0;

      ImGui::RadioButton("Colorido", &enabled, 0);
      ImGui::RadioButton("Preto e branco", &enabled, 1);

      if (enabled == 0) {
        abcg::glClearColor(0.2f, 0.5f, 0.9f, 1);
        m_cat.m_color = glm::vec4{1.0f, 0.69f, 0.3f, 1.0f};
        glm::vec4 asteroid_color{1.0f, 0.0f, 0.0f, 1.0f};

        for (auto &asteroid : m_asteroids.m_asteroids) {
          asteroid.m_color = asteroid_color;
        }
        m_asteroids.m_color_asteroids = asteroid_color;
      } else if (enabled == 1) {
        abcg::glClearColor(0.0f, 0.0f, 0.0f, 1);
        m_cat.m_color = glm::vec4{1.0f, 1.0f, 1.0f, 0};
        glm::vec4 asteroid_color{1.0f, 1.0f, 1.0f, 1.0f};

        for (auto &asteroid : m_asteroids.m_asteroids) {
          asteroid.m_color = asteroid_color;
        }
        m_asteroids.m_color_asteroids = asteroid_color;
      }
    }

    if (m_gameData.m_state == State::GameOver) {
      ImGui::Text("    *Game Over!*");
      ImGui::Button("Jogar Novamente", ImVec2(-1, 50));
      // See also IsItemHovered, IsItemActive, etc
      if (ImGui::IsItemClicked()) {
        restart();
      }
    } else if (m_gameData.m_state == State::Win) {
      ImGui::Text("    *You Win!*");
      ImGui::Button("Jogar Novamente", ImVec2(-1, 50));
      // See also IsItemHovered, IsItemActive, etc
      if (ImGui::IsItemClicked()) {
        restart();
      }
    }

    ImGui::PopFont();
    ImGui::End();
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  abcg::glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLWindow::terminateGL() {
  abcg::glDeleteProgram(m_objectsProgram);
  abcg::glDeleteProgram(m_objectsProgram);

  m_asteroids.terminateGL();
  m_cat.terminateGL();
  m_clouds.terminateGL();
}

void OpenGLWindow::checkCollisions() {
  // Check collision between ship and asteroids
  for (const auto &asteroid : m_asteroids.m_asteroids) {
    const auto asteroidTranslation{asteroid.m_translation};
    const auto distance{
        glm::distance(m_cat.m_translation, asteroidTranslation)};

    if (distance < m_cat.m_scale * 0.9f + asteroid.m_scale * 0.85f) {
      m_gameData.m_state = State::GameOver;
    }
  }

  m_asteroids.m_asteroids.remove_if(
      [](const Asteroids::Asteroid &a) { return a.m_hit; });
}

void OpenGLWindow::checkWinCondition() {
  if (m_rounds == m_total_time && m_gameData.m_state == State::Playing) {
    m_gameData.m_state = State::Win;
  }
}