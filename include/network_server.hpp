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
#include <random>
#include <thread>
#include <mutex>
#include <queue>

// "nn" : nickname
// "so" : search opponent

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
	ENetPeer* m_peer;
	bool m_in_queue;
	bool m_in_game;
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
				if (!std::count(orange.begin(), orange.end(), pos))
					m_board[j][i] = 1;
				else
					m_board[j][i] = 0;
			}
		}
		for (int i{ 0 }; i < 6; ++i)
			card[i] = card_gen(gen);

		// set initial turn
		std::uniform_int_distribution<> turn_gen(0, 1);
		turn = turn_gen(gen);
	}

	std::shared_ptr<Player> m_player[2];

	// -1 : destroyed
	//  0 : orange
	//  1 : banane
	//  2 : empty
	int m_board[8][8] = {
		{2,2,2,2,2,2,2,2},
		{2,2,2,2,2,2,2,2},
		{2,2,2,2,2,2,2,2},
		{2,2,2,2,2,2,2,2},
		{2,2,2,2,2,2,2,2},
		{2,2,2,2,2,2,2,2},
		{2,2,2,2,2,2,2,2},
		{2,2,2,2,2,2,2,2}
	};

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
	int card[6] = {-1,-1,-1,-1,-1,-1};
	int cardOwner[6] = { -1,-1,-1,-1,-1,-1 }; // reference the index of the player in the m_player array

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

inline std::queue<ClientMessage> g_message_queue;
inline std::mutex g_message_mutex;
void message_processing(int tid, bool& server_on);

class NetworkServer
{
	public:

		NetworkServer();
		~NetworkServer();
		void run();
		void shutdown();

	private:

		void matchmaking();

	private:

		bool m_active;
		const unsigned int m_num_threads;
		std::vector<std::thread> m_thread_pool;
		ENetAddress m_address;
		ENetHost* m_server;
		ENetEvent m_event;
		std::vector<std::shared_ptr<Player>> m_player;
		std::vector<Game> m_game;
};

#endif