#ifndef GAME_HPP
#define GAME_HPP

#include <algorithm>
#include <memory>
#include <utility>
#include <enet/enet.h>
#include <mutex>
#include <array>
#include <vector>
#include <glm/glm.hpp>
#include "logic.hpp"

class Game;

struct Player
{
	Player() : m_name(""), m_peer(nullptr), m_in_queue(false), m_in_game(false)
	{}

	Player(std::string name, ENetPeer* peer) : m_name(name), m_peer(peer), m_in_queue(false), m_in_game(false)
	{}

	bool is_in_queue()
	{
		std::unique_lock<std::mutex> lk(m_in_queue_mtx);
		return m_in_queue;
	}

	std::string m_name;
	std::string m_avatar;
	std::shared_ptr<Game> m_game;
	ENetPeer* m_peer;
	bool m_in_queue;
	bool m_in_game;
	std::mutex m_in_queue_mtx;
};

struct Card
{
	enum TYPE
	{
		CONVERSION,
		CONFISCATION,
		CELERITE,
		CHARGE,
		DESORDRE,
		ENTRACTE,
		ENCLUME,
		PETRIFICATION,
		PIEGE,
		RENFORT,
		SOLO,
		VACHETTE
	};

	Card(int t) : type(static_cast<Card::TYPE>(t))
	{
		target[0] = -1;
		target[1] = -1;
	}

	TYPE type;
	int target[2];
};

class Game
{
public:
	struct State
	{
		char turn;
		char winner;
	};
public:
	Game(std::shared_ptr<Player> playerA, std::shared_ptr<Player> playerB);
	std::string cards_to_string();

public:
	std::shared_ptr<Player> m_player_orange;
	std::shared_ptr<Player> m_player_banana;
	Board m_board;
	State m_state;
	std::array<Card, 3> m_orange_cards;
	std::array<Card, 3> m_banana_cards;
};

#endif
