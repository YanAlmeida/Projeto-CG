#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <imgui.h>

#include <random>

#include "abcg.hpp"
#include "asteroids.hpp"
#include "cat.hpp"
#include "clouds.hpp"
#include "starlayers.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
 protected:
  void handleEvent(SDL_Event& event) override;
  void initializeGL() override;
  void paintGL() override;
  void paintUI() override;
  void resizeGL(int width, int height) override;
  void terminateGL() override;

 private:
  GLuint m_starsProgram{};
  GLuint m_objectsProgram{};

  int m_viewportWidth{};
  int m_viewportHeight{};
  int m_rounds{0};
  int m_pedras_desviadas{0};
  int m_mode{0};
  const int m_total_time{60};

  GameData m_gameData;

  Asteroids m_asteroids;
  Cat m_cat;
  StarLayers m_starLayers;
  Clouds m_clouds;

  abcg::ElapsedTimer m_gameTimer;
  abcg::ElapsedTimer m_ScreenTimer;

  ImFont* m_font{};

  std::default_random_engine m_randomEngine;

  void resetKeys();

  void checkCollisions();
  void checkWinCondition();
  void decide_mode(int mode);

  void restart();
  void update();
};

#endif