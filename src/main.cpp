#include "network_server.hpp"

void message_processing(int tid, bool& server_on, NetworkServer& server)
{
	while (server_on)
	{
		ClientMessage client_message;
		g_message_mutex.lock();
		if (!g_message_queue.empty())
		{
			client_message = g_message_queue.front();
			g_message_queue.pop();
		}
		else
		{
			g_message_mutex.unlock();
			continue;
		}
		g_message_mutex.unlock();

		auto& player{ client_message.m_player };
		std::string& data{ client_message.m_message };
		if (data[0] == 'n' && data[1] == 'n') // nickname
		{
			player->m_name = data.substr(2, data.size() - 1);
		}
		else if (data[0] == 'p' && data[1] == 'p') // profile picture
		{
			std::istringstream pp(data.substr(2));
			std::vector<std::string> pp_attr;
			std::string attr;
			while (getline(pp, attr, '.'))
			{
				pp_attr.push_back(attr);
			}
			player->m_avatar.m_gender = std::atoi(pp_attr[0].c_str());
			player->m_avatar.m_hair = std::atoi(pp_attr[1].c_str());
			player->m_avatar.m_eyes = std::atoi(pp_attr[2].c_str());
			player->m_avatar.m_mouth = std::atoi(pp_attr[3].c_str());
			player->m_avatar.m_skin_color = std::atoi(pp_attr[4].c_str());
			player->m_avatar.m_hair_color = std::atoi(pp_attr[5].c_str());
			player->m_avatar.m_eyes_color = std::atoi(pp_attr[6].c_str());
			player->m_avatar.m_data = data.substr(2);
		}
		else if (data == "so") // search opponent
		{
			player->m_in_queue = true;
		}
		else if (data == "sso") // stop search opponent
		{
			player->m_in_queue = false;
		}
		else if (data[0] == 'g' && data[1] == 'c') // game chat
		{
			std::string header0("gc:");
			std::string chat;
			chat += player->m_name;
			chat += " > ";
			chat += data.substr(3);
			std::string header1(std::to_string(data.substr(3).size()) + ":");
			std::string chatMsg = header0 + header1 + chat;
			std::shared_ptr<Game> game{player->m_game};
			g_server_mutex.lock();
			server.send_data(game->m_player[0]->m_peer, chatMsg);
			server.send_data(game->m_player[1]->m_peer, chatMsg);
			g_server_mutex.unlock();
		}
	}
}

int main(int argc, char* argv[])
{
	NetworkServer server;
	
	// pool of threads
	const unsigned int num_threads{ std::thread::hardware_concurrency() };
	std::vector<std::thread> thread_pool;
	for (unsigned int i{ 0 }; i < /*num_threads*/2; ++i)
	{
		thread_pool.emplace_back(message_processing, i + 1, std::ref(server.get_active()), std::ref(server));
	}

	// run server
	server.run();
	
	// join threads
	for (auto& thr : thread_pool) {
		thr.join();
	}
	return 0;
}