#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Right, Left, Down, Up };
enum class State { Initial, Playing, GameOver, Win };

struct GameData {
  State m_state{State::Initial};
  std::bitset<5> m_input;
};

#endif