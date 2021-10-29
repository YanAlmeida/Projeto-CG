#include "openglwindow.hpp"

#include <imgui.h>

#include <string>

#include "abcg.hpp"

void OpenGLWindow::handleEvent(SDL_Event &event) {
  // Keyboard events (Movimentacao do gato via teclado)
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

  // Mouse events (movimentacao do gato através do mouse)
  if (event.type == SDL_MOUSEMOTION) {
    glm::ivec2 mousePosition;
    SDL_GetMouseState(&mousePosition.x, &mousePosition.y);

    glm::vec2 position{
        glm::vec2{(float)mousePosition.x / (m_viewportWidth / 2) - 1,
                  (float)mousePosition.y / (m_viewportHeight / 2) - 1}};

    position.y = -position.y;

    // Define limites para posição do gato
    if (position.x > -(1 - m_cat.m_scale) && position.x < (1 - m_cat.m_scale))
      m_cat.m_translation.x = position.x;
    if (position.y > -(1 - m_cat.m_scale) && position.y < (1 - m_cat.m_scale))
      m_cat.m_translation.y = position.y;
  }
}

void OpenGLWindow::initializeGL() {
  // Nova fonte
  ImGuiIO &io{ImGui::GetIO()};
  const auto filename{getAssetsPath() + "Inconsolata-Medium.ttf"};
  m_font = io.Fonts->AddFontFromFileTTF(filename.c_str(), 40.0f);
  if (m_font == nullptr) {
    throw abcg::Exception{abcg::Exception::Runtime("Cannot load font file")};
  }

  // Programa para renderizar estrelas
  m_starsProgram = createProgramFromFile(getAssetsPath() + "stars.vert",
                                         getAssetsPath() + "stars.frag");
  // Programa para renderizar objetos
  m_objectsProgram = createProgramFromFile(getAssetsPath() + "objects.vert",
                                           getAssetsPath() + "objects.frag");

  abcg::glClearColor(0.2f, 0.5f, 0.9f, 1);

#if !defined(__EMSCRIPTEN__)
  abcg::glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  m_starLayers.initializeGL(m_starsProgram, 25);
  m_clouds.initializeGL(m_objectsProgram, 3);
  m_cat.initializeGL(m_objectsProgram);
  m_asteroids.initializeGL(m_objectsProgram, 1);
}

// Função de restart do jogo
void OpenGLWindow::restart() {
  m_rounds = 0;
  m_pedras_desviadas = 0;
  m_ScreenTimer.restart();
  m_gameTimer.restart();
  m_gameData.m_state = State::Playing;
  m_starLayers.initializeGL(m_starsProgram, 25);
  m_clouds.initializeGL(m_objectsProgram, 3);
  m_cat.initializeGL(m_objectsProgram);
  m_asteroids.initializeGL(m_objectsProgram, 1);
}

void OpenGLWindow::update() {
  const float deltaTime{static_cast<float>(getDeltaTime())};

  m_cat.update(m_gameData, deltaTime);
  m_asteroids.update(deltaTime, &m_pedras_desviadas);
  float interval;

  // define parametros para ordenacao de asteroids gerados
  std::uniform_real_distribution<float> m_randomDist{-1.0f, 1.0f};
  int ordenation = signbit(m_randomDist(m_randomEngine));
  int starting_point = 1;
  if (ordenation) starting_point = -1;

  if (m_gameData.m_state == State::Playing) {
    // controla tamanho do intervalo de acordo com tempo passado, baseado no
    // tempo total definido no arquivo .hpp
    if (m_total_time - m_ScreenTimer.elapsed() < m_total_time / 2.0f &&
        m_total_time - m_ScreenTimer.elapsed() > m_total_time / 6.0f) {
      interval = 0.75f;
    } else if (m_total_time - m_ScreenTimer.elapsed() < m_total_time / 6.0f) {
      interval = 1.2f;
    } else {
      interval = 1.0f;
    }

    // cria asteroides a cada intervalo, modificando sua velocidade segundo o
    // tempo
    if (m_gameTimer.elapsed() > interval) {
      m_gameTimer.restart();
      std::generate_n(std::back_inserter(m_asteroids.m_asteroids), 1, [&]() {
        float inverse_velocity =
            (float)(m_total_time - m_ScreenTimer.elapsed()) /
            (m_total_time / 5.0f);
        return m_asteroids.createAsteroid(
            glm::vec2{m_randomDist(m_randomEngine), starting_point},
            inverse_velocity, ordenation);
      });
    }

    checkCollisions();
    checkWinCondition();
  } else if (m_gameTimer.elapsed() > 5.0f) {
    m_gameTimer.restart();
    std::generate_n(std::back_inserter(m_asteroids.m_asteroids), 3, [&]() {
      return m_asteroids.createAsteroid(
          glm::vec2{m_randomDist(m_randomEngine), starting_point}, 5.5f,
          ordenation);
    });
  }
}

void OpenGLWindow::paintGL() {
  update();

  abcg::glClear(GL_COLOR_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  m_starLayers.paintGL();
  m_clouds.paintGL();
  m_asteroids.paintGL();
  m_cat.paintGL(m_gameData);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();
  // texto que aparece durante o game
  if (m_gameData.m_state == State::Playing) {
    std::string s = std::to_string(m_pedras_desviadas);
    char const *pchar = s.c_str();

    std::string s2 = std::to_string(m_total_time - m_ScreenTimer.elapsed());
    char const *pchar2 = s2.c_str();

    // definições do imgui
    const auto size{ImVec2(720, 150)};
    const auto position{ImVec2(0, 0)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);

    ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                           ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoInputs};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font);
    
    if (m_mode == 0){
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,0,0,255));
    ImGui::Columns(2);
    ImGui::SetColumnWidth(1, 100);
    ImGui::Text("Pedras desviadas:");
    ImGui::Text("Tempo restante:");
    ImGui::NextColumn();
    ImGui::SetColumnWidth(1, 85);
    ImGui::Text(pchar);
    ImGui::Text(pchar2);
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::End();
    }else{
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,255,255,255));
    ImGui::Columns(2);
    ImGui::SetColumnWidth(1, 100);
    ImGui::Text("Pedras desviadas:");
    ImGui::Text("Tempo restante:");
    ImGui::NextColumn();
    ImGui::SetColumnWidth(1, 85);
    ImGui::Text(pchar);
    ImGui::Text(pchar2);
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::End();
    }

  } else {
    const auto size{ImVec2(380, 260)};
    const auto position{ImVec2((m_viewportWidth - size.x) / 2.0f,
                               (m_viewportHeight - size.y) / 2.0f)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                           ImGuiWindowFlags_NoTitleBar};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font);

    // Define texto e botoes para cada estado de jogo (inicial, win e game over)
    if (m_gameData.m_state == State::Initial) {
      ImGui::Text("    *Cat run!*");
      ImGui::RadioButton("Dia", &m_mode, 0);
      ImGui::RadioButton("Noite", &m_mode, 1);

      decide_mode(m_mode);

      ImGui::Button("Iniciar", ImVec2(-1, 50));
      // Criação condição botão
      if (ImGui::IsItemClicked()) {
        restart();
      }
    }

    if (m_gameData.m_state == State::GameOver) {
      ImGui::Text("    *Game Over!*");

      ImGui::RadioButton("Dia", &m_mode, 0);
      ImGui::RadioButton("Noite", &m_mode, 1);
      decide_mode(m_mode);

      ImGui::Button("Jogar Novamente", ImVec2(-1, 50));
      // Criação condição botão
      if (ImGui::IsItemClicked()) {
        restart();
      }
    } else if (m_gameData.m_state == State::Win) {
      ImGui::Text("    *You Win!*");

      ImGui::RadioButton("Dia", &m_mode, 0);
      ImGui::RadioButton("Noite", &m_mode, 1);

      decide_mode(m_mode);
      ImGui::Button("Jogar Novamente", ImVec2(-1, 50));
      // Criação condição botão
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
  abcg::glDeleteProgram(m_starsProgram);
  abcg::glDeleteProgram(m_objectsProgram);

  m_asteroids.terminateGL();
  m_cat.terminateGL();
  m_clouds.terminateGL();
  m_starLayers.terminateGL();
}

// Funcao para checar colisao entre o gato e os asteroides e para destruir
// asteroides que sairem da tela
void OpenGLWindow::checkCollisions() {
  // Verifica a colisão entre gato e asteróides
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

// Funcao para checar se o tempo total de jogo passou (vitoria)
void OpenGLWindow::checkWinCondition() {
  if (m_ScreenTimer.elapsed() >= m_total_time &&
      m_gameData.m_state == State::Playing) {
    m_gameData.m_state = State::Win;
  }
}

// Funcao para decidir modo (dia ou noite)
void OpenGLWindow::decide_mode(int mode) {
  glm::vec4 cat_color;
  glm::vec4 asteroid_color;
  glm::vec4 cloud_color;

  if (mode == 0) {
    abcg::glClearColor(0.2f, 0.5f, 0.9f, 1);
    cat_color = glm::vec4{1.0f, 0.69f, 0.3f, 1.0f};
    asteroid_color = glm::vec4{0.90f, 0.4f, 0.5f, 1.0f};
    cloud_color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
  } else {
    abcg::glClearColor(0.0f, 0.0f, 0.0f, 1);
    cat_color = glm::vec4{1.0f, 1.0f, 1.0f, 0};
    asteroid_color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    cloud_color = glm::vec4{0.45f, 0.45f, 0.45f, 0.45f};
  }

  m_cat.m_color = cat_color;
  for (auto &asteroid : m_asteroids.m_asteroids) {
    asteroid.m_color = asteroid_color;
  }
  m_asteroids.m_color_asteroids = asteroid_color;

  for (auto &cloud : m_clouds.m_clouds) {
    cloud.m_color = cloud_color;
  }
  m_clouds.m_cloud_color = cloud_color;
}