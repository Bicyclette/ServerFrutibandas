#include "network_server.hpp"

NetworkServer::NetworkServer() :
	m_active(true),
	m_server(nullptr)
{
	if (enet_initialize() != 0)
	{
		throw std::runtime_error("An error occured while initializing ENet !");
	}
	atexit(enet_deinitialize);

	m_address.host = ENET_HOST_ANY;
	m_address.port = 7777;

	m_server = enet_host_create(&m_address, 2, 1, 0, 0);
	if (m_server == nullptr)
	{
		enet_deinitialize();
		throw std::runtime_error("Error while trying to create the network server !");
	}
}

NetworkServer::~NetworkServer()
{
	enet_host_destroy(m_server);
	for (auto& p : m_player)
	{
		enet_peer_disconnect(p->m_peer, 0);
	}
}

bool NetworkServer::is_active()
{
	m_active_mtx.lock();
	return m_active;
	m_active_mtx.unlock();
}

void NetworkServer::run()
{
	while (m_active)
	{
		while (enet_host_service(m_server, &m_event, 0) > 0)
		{
			if (m_event.type == ENET_EVENT_TYPE_CONNECT)
			{
				m_player.emplace_back(std::make_shared<Player>("", m_event.peer));
			}
			else if (m_event.type == ENET_EVENT_TYPE_RECEIVE)
			{
				std::string message(reinterpret_cast<char*>(m_event.packet->data));
				for (int i{ 0 }; i < m_player.size(); ++i)
				{
					auto& p{ m_player[i] };
					ENetAddress player_address = p->m_peer->address;
					ENetAddress peer_address = m_event.peer->address;
					if (player_address.host == peer_address.host && player_address.port == peer_address.port)
					{
						g_message_queue_mtx.lock();
						g_message_queue.emplace(p, message);
						g_message_queue_mtx.unlock();
					}
				}
			}
			else if (m_event.type == ENET_EVENT_TYPE_DISCONNECT)
			{
				for (int i{0}; i < m_player.size(); ++i)
				{
					auto& p{m_player[i]};
					ENetAddress player_address = p->m_peer->address;
					ENetAddress peer_address = m_event.peer->address;
					if (player_address.host == peer_address.host && player_address.port == peer_address.port)
					{
						std::cout << m_player[i]->m_name << " has been disconnected !" << std::endl;
						if (p->m_in_game)
						{
							std::string dc("dc");
							g_message_queue_mtx.lock();
							g_message_queue.emplace(p, dc);
							g_message_queue_mtx.unlock();
						}
						m_player.erase(m_player.begin()+i);
						break;
					}
				}
			}
		}

		matchmaking();
	}
}

void NetworkServer::shutdown()
{
	m_active_mtx.lock();
	m_active = false;
	m_active_mtx.unlock();
}

void NetworkServer::send_data(ENetPeer* peer, std::string data)
{
	ENetPacket* packet = enet_packet_create(data.c_str(), data.size() + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

void NetworkServer::matchmaking()
{
	int opponents[2] = { -1,-1 };
	for (int i{ 0 }; i < m_player.size(); ++i)
	{
		if (m_player[i]->m_in_queue)
		{
			if (opponents[0] == -1)
				opponents[0] = i;
			else if (opponents[1] == -1)
				opponents[1] = i;
		}
		if (opponents[0] != -1 && opponents[1] != -1)
		{
			m_player[opponents[0]]->m_in_queue_mtx.lock();
			m_player[opponents[0]]->m_in_queue = false;
			m_player[opponents[0]]->m_in_queue_mtx.unlock();
			m_player[opponents[0]]->m_in_game = true;

			m_player[opponents[1]]->m_in_queue_mtx.lock();
			m_player[opponents[1]]->m_in_queue = false;
			m_player[opponents[1]]->m_in_queue_mtx.unlock();
			m_player[opponents[1]]->m_in_game = true;

			m_game.push_back(std::make_shared<Game>(m_player[opponents[0]], m_player[opponents[1]]));
			m_player[opponents[0]]->m_game = m_game[m_game.size()-1];
			m_player[opponents[1]]->m_game = m_game[m_game.size()-1];
			
			// send initial data to players
			std::string cards = m_game[m_game.size() - 1]->cards_to_string();
			std::string board = m_game[m_game.size() - 1]->m_board.to_string();
			std::shared_ptr<Player> pOrange = m_game[m_game.size() - 1]->m_player_orange;
			std::shared_ptr<Player> pBanana = m_game[m_game.size() - 1]->m_player_banana;
			if (m_game[m_game.size() - 1]->m_state.turn == 'b') {
				send_data(pBanana->m_peer, "gs:nn:" + pOrange->m_name + ":pp:" + pOrange->m_avatar + ":team:1" + ":b:" + board + ":turn:1:c:" + cards);
				send_data(pOrange->m_peer, "gs:nn:" + pBanana->m_name + ":pp:" + pBanana->m_avatar + ":team:0" + ":b:" + board + ":turn:1:c:" + cards);
			}
			else {
				send_data(pOrange->m_peer, "gs:nn:" + pBanana->m_name + ":pp:" + pBanana->m_avatar + ":team:0" + ":b:" + board + ":turn:0:c:" + cards);
				send_data(pBanana->m_peer, "gs:nn:" + pOrange->m_name + ":pp:" + pOrange->m_avatar + ":team:1" + ":b:" + board + ":turn:0:c:" + cards);
			}

			// reset
			opponents[0] = -1;
			opponents[1] = -1;
		}
	}
}
