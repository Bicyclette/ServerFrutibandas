#ifndef NETWORK_SERVER_HPP
#define NETWORK_SERVER_HPP

#include <iostream>
#include <memory>
#include <utility>
#include <glm/glm.hpp>
#include <exception>
#include <enet/enet.h>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <thread>
#include <sstream>
#include <mutex>
#include <queue>

struct Game;

// "nn"  : nickname
// "pp"  : profile picture
// "so"  : search opponent
// "sso" : stop search opponent
// "gc:" : game chat
// eye colors

struct Avatar
{
	int m_gender;		// 0 : male, 1 : female
	int m_hair;			// [0,7] : [mixte, herisson, decoiffe, arriere, meche avant, mi-long, frange, au_bol, ponytail]
	int m_eyes;			// [0,4] : [manga, amande, gros, egypte, mascara]
	int m_mouth;		// [0,2] : [petite, moyenne, grande]
	int m_skin_color;	// [0,9] : [pale1, pale2, pale3, mc1, mc2, mc3, m1, m2, m3, m4]
	int m_hair_color;	// [0,12] : [rouge, orange, j1, j2, v1, v2, b1, b2, b3, pourpre, violet, rose, rose_bonbon]
	int m_eyes_color;	// [0,6] : [jaune, marron, rouge, violet, rose, bleu, vert]
	std::string m_data;	// all above data for network
};

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

struct Fruit
{
	// >>>>>>>>>> methods
	Fruit() : m_type(-1), m_petrified(false) {}
	// >>>>>>>>>> properties
	int m_type;	// -1 = none, 0 = orange, 1 = banane
	bool m_petrified;
};

struct Tile
{
	// >>>>>>>>>> methods
	Tile() : m_alive(true), m_trap(false) {}

	// >>>>>>>>>> properties
	bool m_alive;
	bool m_trap;
};

struct Board
{
	int orange_count()
	{
		int count{ 0 };
		for (int i{ 0 }; i < 8; ++i) {
			for (int j{ 0 }; j < 8; ++j) {
				count += (m_fruit[j][i].m_type == 0) ? 1 : 0;
			}
		}
		return count;
	}

	int banane_count()
	{
		int count{ 0 };
		for (int i{ 0 }; i < 8; ++i) {
			for (int j{ 0 }; j < 8; ++j) {
				count += (m_fruit[j][i].m_type == 1) ? 1 : 0;
			}
		}
		return count;
	}

	Tile m_tile[8][8];
	Fruit m_fruit[8][8];
};

struct Game
{
	Game(std::shared_ptr<Player> p1, std::shared_ptr<Player> p2)
	{
		m_player[0] = p1;
		m_player[1] = p2;

		// create board and set of cards
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> fruit_pos_gen(0, 7);
		std::uniform_int_distribution<> card_gen(0, 11);
		std::vector<glm::vec2> orange;
		std::vector<glm::vec2> banane;
		for (int i{ 0 }; i < 32; ++i)
		{
			glm::vec2 fruit_pos;
			do
			{
				fruit_pos = glm::vec2(fruit_pos_gen(gen), fruit_pos_gen(gen));
			} while (std::count(orange.begin(), orange.end(), fruit_pos) > 0);
			orange.push_back(fruit_pos);
		}
		for (int i{ 0 }; i < 8; ++i)
		{
			for (int j{ 0 }; j < 8; ++j)
			{
				glm::vec2 pos = glm::vec2(j, i);
				if (!std::count(orange.begin(), orange.end(), pos)) {
					m_board.m_fruit[j][i].m_type = 1; // banane
				}
				else {
					m_board.m_fruit[j][i].m_type = 0; // orange
				}
			}
		}
		for (int i{ 0 }; i < 6; ++i)
		{
			int card_id;
			do
			{
				card_id = card_gen(gen);
			} while (std::count(card.begin(), card.end(), card_id) > 0);
			card[i] = card_id;
		}

		// set initial turn
		std::uniform_int_distribution<> turn_gen(0, 1);
		turn = turn_gen(gen);
	}

	std::shared_ptr<Player> m_player[2];
	Board m_board;
	// -1 => undefined
	//  0 => enclume
	//  1 => celerite
	//  2 => confiscation
	//  3 => renfort
	//  4 => desordre
	//  5 => petrification
	//  6 => vachette
	//  7 => conversion
	//  8 => charge
	//  9 => entracte
	// 10 => solo
	// 11 => piege
	std::array<int, 6> card = {-1,-1,-1,-1,-1,-1};
	int cardOwner[6] = { 0,0,0,1,1,1 }; // reference the index of the player in the m_player array
	int turn{-1}; // reference the index of the player in the m_player array
	int remaining_time[2] = {360, 360}; // in seconds (6 min)
	int winner{-1};
};

struct ClientMessage
{
	ClientMessage(){}
	ClientMessage(std::shared_ptr<Player> player, std::string& message)
	{
		m_player = player;
		m_message = message;
	}

	std::shared_ptr<Player> m_player;
	std::string m_message;
};

class NetworkServer
{
	public:

		NetworkServer();
		~NetworkServer();
		bool& get_active() { return m_active; }
		void run();
		void shutdown();
		void send_data(ENetPeer* peer, std::string data);

	private:

		void matchmaking();

	private:

		bool m_active;
		ENetAddress m_address;
		ENetHost* m_server;
		ENetEvent m_event;
		std::vector<std::shared_ptr<Player>> m_player;
		std::vector<std::shared_ptr<Game>> m_game;
};

inline std::queue<ClientMessage> g_message_queue;
inline std::mutex g_message_mutex;
inline std::mutex g_server_mutex;

#endif