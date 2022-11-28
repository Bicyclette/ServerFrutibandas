#include "network_server.hpp"

NetworkServer::NetworkServer(enet_uint16 port) :
	m_active(true),
	m_server(nullptr)
{
	if (enet_initialize() != 0)
	{
		throw std::runtime_error("An error occured while initializing ENet !");
	}
	atexit(enet_deinitialize);

	m_address.host = ENET_HOST_ANY;
	m_address.port = port;

	m_server = enet_host_create(&m_address, 1024, 1, 0, 0);
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

void NetworkServer::set_port(enet_uint16 port)
{
	enet_host_destroy(m_server);
	m_address.port = port;
	
	m_server = enet_host_create(&m_address, 1024, 1, 0, 0);
	if (m_server == nullptr)
	{
		enet_deinitialize();
		throw std::runtime_error("Error while trying to create the network server !");
	}
}

bool NetworkServer::is_active()
{
	std::unique_lock<std::mutex> lk(m_active_mtx);
	return m_active;
}

void NetworkServer::run()
{
	while (m_active)
	{
		if (enet_host_service(m_server, &m_event, 0) > 0)
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
						std::cout << p->m_name << " is now disconnected." << std::endl;
						if (p->m_in_game)
						{
							std::string dc("dc");
							g_message_queue_mtx.lock();
							g_message_queue.emplace(p, dc);
							g_message_queue_mtx.unlock();
						}
						m_player.erase(m_player.begin()+i);
						size_t num_players = get_player_count();
						std::cout << "Number of connected players = " << num_players << std::endl;
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

size_t NetworkServer::get_player_count() const
{
	return m_player.size();
}

void NetworkServer::matchmaking()
{
	int opponents[2] = { -1,-1 };
	for (int i{ 0 }; i < m_player.size(); ++i)
	{
		if (m_player[i]->is_in_queue())
		{
			if (opponents[0] == -1) {
				opponents[0] = i;
			}
			else if(m_player[opponents[0]]->is_in_queue())
			{
				if (opponents[1] == -1) {
					opponents[1] = i;
				}
			}
			else {
				opponents[0] = -1;
			}
		}
		if (opponents[0] != -1 && opponents[1] != -1)
		{
			std::shared_ptr<Player> p1 = m_player[opponents[0]];
			std::shared_ptr<Player> p2 = m_player[opponents[1]];

			if (!p1->is_in_queue() || !p2->is_in_queue())
			{
				return;
			}

			p1->m_in_queue_mtx.lock();
			p1->m_in_queue = false;
			p1->m_in_queue_mtx.unlock();
			p1->m_in_game = true;

			p2->m_in_queue_mtx.lock();
			p2->m_in_queue = false;
			p2->m_in_queue_mtx.unlock();
			p2->m_in_game = true;

			std::shared_ptr<Game> g = std::make_shared<Game>(p1, p2);
			m_game.push_back(g);
			p1->m_game = g;
			p2->m_game = g;
			
			// send initial data to players
			std::string cards = g->cards_to_string();
			std::string board = g->m_board.to_string();
			std::shared_ptr<Player> pOrange = g->m_player_orange;
			std::shared_ptr<Player> pBanana = g->m_player_banana;
			if (g->m_state.turn == 'b') {
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
			return;
		}
	}
}
