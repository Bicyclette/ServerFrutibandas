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
			player->m_in_queue = true;
		}
		else if (message == "sso")
		{
			player->m_in_queue = false;
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