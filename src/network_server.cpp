#include "network_server.hpp"

NetworkServer::NetworkServer() :
	m_active(true),
	m_server(nullptr)
{
	if (enet_initialize() != 0)
	{
		throw std::exception("An error occured while initializing ENet !");
	}
	atexit(enet_deinitialize);

	m_address.host = ENET_HOST_ANY;
	m_address.port = 7777;

	m_server = enet_host_create(&m_address, 2, 1, 0, 0);
	if (m_server == nullptr)
	{
		enet_deinitialize();
		throw std::exception("Error while trying to create the network server !");
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

void NetworkServer::run()
{
	while (m_active)
	{
		while (enet_host_service(m_server, &m_event, 0) > 0)
		{
			if (m_event.type == ENET_EVENT_TYPE_CONNECT)
			{
				m_player.emplace_back(std::make_shared<Player>("", m_event.peer));
				std::cout << "A new client connected from ";
				std::cout << m_event.peer->address.host << ':';
				std::cout << m_event.peer->address.port;
				std::cout << std::endl;
			}
			else if (m_event.type == ENET_EVENT_TYPE_RECEIVE)
			{
				std::string message(reinterpret_cast<char*>(m_event.packet->data));
				std::cout << message << std::endl;
				for (int i{ 0 }; i < m_player.size(); ++i)
				{
					auto& p{ m_player[i] };
					ENetAddress player_address = p->m_peer->address;
					ENetAddress peer_address = m_event.peer->address;
					if (player_address.host == peer_address.host && player_address.port == peer_address.port)
					{
						g_message_mutex.lock();
						g_message_queue.emplace(p, message);
						g_message_mutex.unlock();
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
	m_active = false;
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
			m_player[opponents[0]]->m_in_queue = false;
			m_player[opponents[0]]->m_in_game = true;
			m_player[opponents[1]]->m_in_queue = false;
			m_player[opponents[1]]->m_in_game = true;
			m_game.push_back(std::make_shared<Game>(m_player[opponents[0]], m_player[opponents[1]]));
			m_player[opponents[0]]->m_game = m_game[m_game.size()-1];
			m_player[opponents[1]]->m_game = m_game[m_game.size()-1];
			
			// send initial game data (players name, players profile picture, game state)
			std::string orange_data;
			// orange player name
			orange_data += m_player[opponents[0]]->m_name + ":";
			// orange player profile picture
			orange_data += m_player[opponents[0]]->m_avatar.m_data;

			std::string banane_data;
			// banane player name
			banane_data += m_player[opponents[1]]->m_name + ":";
			// banane player profile picture
			banane_data += m_player[opponents[1]]->m_avatar.m_data;

			// turn
			std::string turn = std::to_string(m_game[m_game.size() - 1]->turn);

			// board
			std::string board("");
			for (int i{ 0 }; i < 8; ++i)
			{
				for (int j{ 0 }; j < 8; ++j)
				{
					int fruit = m_game[m_game.size() - 1]->m_board.m_fruit[j][i].m_type;
					board += std::to_string(fruit);
					if ((i * 8 + j) < 63) {
						board += ".";
					}
				}
			}

			// set of cards => orange
			std::string orange_cards("");
			orange_cards += std::to_string(m_game[m_game.size() - 1]->card[0]) + ".";
			orange_cards += std::to_string(m_game[m_game.size() - 1]->card[1]) + ".";
			orange_cards += std::to_string(m_game[m_game.size() - 1]->card[2]);

			// set of cards => banane
			std::string banane_cards("");
			banane_cards += std::to_string(m_game[m_game.size() - 1]->card[3]) + ".";
			banane_cards += std::to_string(m_game[m_game.size() - 1]->card[4]) + ".";
			banane_cards += std::to_string(m_game[m_game.size() - 1]->card[5]);

			// send for orange player
			std::string dataOrange("g0:");
			dataOrange += "0:"; // fruit identity
			dataOrange += turn + ":";
			dataOrange += banane_data + ":";
			dataOrange += orange_cards + ":";
			dataOrange += board;
			send_data(m_player[opponents[0]]->m_peer, dataOrange);

			// send for banane player
			std::string dataBanane("g0:");
			dataBanane += "1:"; // fruit identity
			dataBanane += turn + ":";
			dataBanane += orange_data + ":";
			dataBanane += banane_cards + ":";
			dataBanane += board;
			send_data(m_player[opponents[1]]->m_peer, dataBanane);

			// reset
			opponents[0] = -1;
			opponents[1] = -1;
		}
	}
}