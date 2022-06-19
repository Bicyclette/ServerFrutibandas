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
	enum class TYPE
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

	Card(int t) : type(static_cast<TYPE>(t)) {}

	int getType()
	{
		return static_cast<int>(type);
	}

	friend bool operator==(const Card& a, const Card& b)
	{
		return a.type == b.type;
	}

	TYPE type;
	std::shared_ptr<Player> owner;
	glm::ivec2 target;
};

class Game
{
public:
	struct State
	{
		Fruit turn;
		Fruit winner;
		bool choose_card;
	};
public:
	Game(std::shared_ptr<Player> playerA, std::shared_ptr<Player> playerB);
	std::string get_free_cards();
public:
	std::shared_ptr<Player> m_player_orange;
	std::shared_ptr<Player> m_player_banana;
	Board m_board;
	State m_state;
	std::array<Card, 12> m_cards;
	std::array<Card, 3> m_orange_cards;
	std::array<Card, 3> m_banana_cards;
};

#endif