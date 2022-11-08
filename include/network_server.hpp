#ifndef NETWORK_SERVER_HPP
#define NETWORK_SERVER_HPP

#include <iostream>
#include <memory>
#include <utility>
#include <exception>
#include <enet/enet.h>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include "game.hpp"

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

		NetworkServer(enet_uint16 port = 7777);
		~NetworkServer();
		void set_port(enet_uint16 port);
		bool is_active();
		void run();
		void shutdown();
		void send_data(ENetPeer* peer, std::string data);
		size_t get_player_count() const;

	private:

		void matchmaking();

	private:

		bool m_active;
		std::mutex m_active_mtx;
		ENetAddress m_address;
		ENetHost* m_server;
		ENetEvent m_event;
		std::vector<std::shared_ptr<Player>> m_player;

	public:
		std::queue<ClientMessage> g_message_queue;
		std::mutex g_message_queue_mtx;
		std::vector<std::shared_ptr<Game>> m_game;
};

// COMMUNICATION >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline std::mutex g_server_mtx;

#endif
