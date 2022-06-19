#include "network_server.hpp"

NetworkServer g_server;

void message_processing(int tid)
{
	while (true)
	{
		ClientMessage client_message;
		g_server.g_message_queue_mtx.lock();
		if (!g_server.g_message_queue.empty())
		{
			client_message = g_server.g_message_queue.front();
			g_server.g_message_queue.pop();
		}
		else
		{
			g_server.g_message_queue_mtx.unlock();
			continue;
		}
		g_server.g_message_queue_mtx.unlock();

		auto& player{ client_message.m_player };
		std::string& message{ client_message.m_message };

		if (message == "so")
		{
			player->m_in_queue_mtx.lock();
			player->m_in_queue = true;
			player->m_in_queue_mtx.unlock();
		}
		else if (message == "sso")
		{
			player->m_in_queue_mtx.lock();
			player->m_in_queue = false;
			player->m_in_queue_mtx.unlock();
		}
		else if (message[0] == 'n' && message[1] == 'n')
		{
			player->m_name = message.substr(3);
		}
		else if (message[0] == 'p' && message[1] == 'p')
		{
			player->m_avatar = message.substr(3);
		}
		else if (message[0] == 'g' && message[1] == 'u')
		{
			std::shared_ptr<Game> game = player->m_game;
			// notify the other player that his opponent gave up
			if (player == player->m_game->m_player_banana)
			{
				g_server.send_data(player->m_game->m_player_orange->m_peer, "gu");
			}
			else
			{
				g_server.send_data(player->m_game->m_player_banana->m_peer, "gu");
			}
			// reset both player's status
			game->m_player_banana->m_in_game = false;
			game->m_player_banana->m_game.reset();
			game->m_player_orange->m_in_game = false;
			game->m_player_orange->m_game.reset();
			// destroy game instance
			g_server_mtx.lock();
			auto pos = std::find(g_server.m_game.begin(), g_server.m_game.end(), game);
			g_server.m_game.erase(pos);
			g_server_mtx.unlock();
		}
		else if (message[0] == 'd' && message[1] == 'c')
		{
			std::shared_ptr<Game> game = player->m_game;
			// notify the other player that his opponent has been disconnected
			if (player == player->m_game->m_player_banana)
			{
				g_server.send_data(player->m_game->m_player_orange->m_peer, "dc");
				game->m_player_orange->m_in_game = false;
				game->m_player_orange->m_game.reset();
			}
			else
			{
				g_server.send_data(player->m_game->m_player_banana->m_peer, "dc");
				game->m_player_banana->m_in_game = false;
				game->m_player_banana->m_game.reset();
			}
			// destroy game instance
			g_server_mtx.lock();
			auto pos = std::find(g_server.m_game.begin(), g_server.m_game.end(), game);
			g_server.m_game.erase(pos);
			g_server_mtx.unlock();
		}
		else if (message[0] == 'g' && message[1] == 'c')
		{
			std::cout << message << std::endl;
			std::string data = "gc:" + player->m_name + " > " + message.substr(3);
			g_server.send_data(player->m_game->m_player_banana->m_peer, data);
			g_server.send_data(player->m_game->m_player_orange->m_peer, data);
		}
	}
}

int main(int argc, char* argv[])
{	
	// pool of threads
	const unsigned int num_threads{ std::thread::hardware_concurrency() };
	std::vector<std::thread> thread_pool;
	for (unsigned int i{ 0 }; i < /*num_threads*/2; ++i)
	{
		thread_pool.emplace_back(message_processing, i + 1);
	}

	// run server
	g_server.run();
	
	// join threads
	for (auto& thr : thread_pool) {
		thr.join();
	}
	return 0;
}