#ifndef GAME_HPP
#define GAME_HPP

#include <memory>
#include <utility>
#include <random>
#include <enet/enet.h>
#include "avatar.hpp"

class Game;

struct Player
{
	Player() :
		m_name(""),
		m_peer(nullptr),
		m_in_queue(false),
		m_in_game(false)
	{}

	Player(std::string name, ENetPeer* peer) :
		m_name(name),
		m_peer(peer),
		m_in_queue(false),
		m_in_game(false)
	{}

	std::string m_name;
	Avatar m_avatar;
	std::shared_ptr<Game> m_game;
	ENetPeer* m_peer;
	bool m_in_queue;
	bool m_in_game;
};

class Game
{
public:
	Game(std::shared_ptr<Player> playerA, std::shared_ptr<Player> playerB);

public:
	std::shared_ptr<Player> m_player_orange;
	std::shared_ptr<Player> m_player_banana;
};

#endif