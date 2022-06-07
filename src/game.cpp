#include "game.hpp"

Game::Game(std::shared_ptr<Player> playerA, std::shared_ptr<Player> playerB)
{
	m_player_orange = playerA;
	m_player_orange = playerB;
}
