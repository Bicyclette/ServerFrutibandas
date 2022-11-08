#include "network_server.hpp"

NetworkServer g_server;

bool isNumber(const std::string& s)
{
	for(const char& c : s)
	{
		if(std::isdigit(c) == 0){
			return false;
		}
	}
	return true;
}

void commands()
{
	std::string cmd;
	bool check_cmd = true;
	while(check_cmd)
	{
		std::cout << ">";
		std::getline(std::cin, cmd);
		if(cmd == "Q") // close server
		{
			check_cmd = false;
			g_server_mtx.lock();
			g_server.shutdown();
			g_server_mtx.unlock();
			std::cout << "CIAO ! ;)" << std::endl;
		}
		else if(cmd == "GC") // request number of ongoing games
		{
			g_server_mtx.lock();
			size_t num_games = g_server.m_game.size();
			g_server_mtx.unlock();
			std::cout << "Number of ongoing games = " << num_games << std::endl;
		}
		else if(cmd == "PC") // request number of connected players
		{
			g_server_mtx.lock();
			size_t num_players = g_server.get_player_count();
			g_server_mtx.unlock();
			std::cout << "Number of connected players = " << num_players << std::endl;
		}
		else
		{
			std::cout << "ERROR::UNKNOWN_COMMAND (" << cmd << ")" << std::endl;
		}
	}
}

void message_processing(int tid)
{
	while (g_server.is_active())
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
			// destroy game instance
			g_server_mtx.lock();
			auto pos = std::find(g_server.m_game.begin(), g_server.m_game.end(), game);
			g_server.m_game.erase(pos);
			g_server_mtx.unlock();
			// reset both player's status
			game->m_player_banana->m_in_game = false;
			game->m_player_banana->m_game.reset();
			game->m_player_orange->m_in_game = false;
			game->m_player_orange->m_game.reset();
		}
		else if (message[0] == 'd' && message[1] == 'c')
		{
			std::shared_ptr<Game> game = player->m_game;
			// destroy game instance
			g_server_mtx.lock();
			auto pos = std::find(g_server.m_game.begin(), g_server.m_game.end(), game);
			g_server.m_game.erase(pos);
			g_server_mtx.unlock();
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
		}
		else if (message[0] == 'g' && message[1] == 'c')
		{
			std::string data = "gc:" + player->m_name + " > " + message.substr(3);
			g_server.send_data(player->m_game->m_player_banana->m_peer, data);
			g_server.send_data(player->m_game->m_player_orange->m_peer, data);
		}
		else if (message[0] == 'm' && message[1] == 'v')
		{
			g_server.send_data(player->m_game->m_player_orange->m_peer, message);
			g_server.send_data(player->m_game->m_player_banana->m_peer, message);
		}
		else if (message[0] == 'w' && message[1] == 'i' && message[2] == 'n')
		{
			std::shared_ptr<Game> game = player->m_game;
			std::string end_game("end");
			if (player == player->m_game->m_player_banana)
			{
				g_server.send_data(player->m_game->m_player_orange->m_peer, end_game);
			}
			else
			{
				g_server.send_data(player->m_game->m_player_banana->m_peer, end_game);
			}
			// destroy game instance
			g_server_mtx.lock();
			auto pos = std::find(g_server.m_game.begin(), g_server.m_game.end(), game);
			g_server.m_game.erase(pos);
			g_server_mtx.unlock();
			// reset both player's status
			game->m_player_banana->m_in_game = false;
			game->m_player_banana->m_game.reset();
			game->m_player_orange->m_in_game = false;
			game->m_player_orange->m_game.reset();
		}
		else if (message[0] == 'c' && message[1] == 'a' && message[2] == 'r' && message[3] == 'd')
		{
			// ignore useless data
			message = message.substr(5);
			// get card identifier
			int next_token = message.find_first_of('.');
			std::string card_id_str = message.substr(0, next_token);
			int card_id = std::atoi(card_id_str.data());
			
			// get card index in owner's array
			message = message.substr(next_token + 1);
			next_token = message.find_first_of('.');
			std::string card_index = message.substr(0, next_token);
			
			// if card has an effect delayed towards enemy, get to which player it has an effect on
			std::string effect_destination;
			if (card_id == 4) {
				message = message.substr(next_token + 1);
				effect_destination = message.substr(0);
			}
			// else if card is reinforcement (spawn up to 3 bandas)
			std::string reinforcement_data;
			if (card_id == 9) {
				message = message.substr(next_token + 1);
				reinforcement_data = message.substr(0);
			}
			// else if targeted card
			std::string target;
			if (card_id == 0 || card_id == 6 || card_id == 7 || card_id == 8 || card_id == 10 || card_id == 11) {
				message = message.substr(next_token + 1);
				target = message.substr(0);
			}

			// generic card data
			std::string card_data = effect_destination + reinforcement_data + target;
			if (card_id < 10) {
				card_id_str = "0" + card_id_str;
			}
			
			if (player == player->m_game->m_player_banana)
			{
				g_server.send_data(player->m_game->m_player_orange->m_peer, "card:" + card_id_str + ".0." + card_index + "." + card_data);
				g_server.send_data(player->m_game->m_player_banana->m_peer, "card:" + card_id_str + ".1." + card_index + "." + card_data);
			}
			else
			{
				g_server.send_data(player->m_game->m_player_banana->m_peer, "card:" + card_id_str + ".0." + card_index + "." + card_data);
				g_server.send_data(player->m_game->m_player_orange->m_peer, "card:" + card_id_str + ".1." + card_index + "." + card_data);
			}
		}
	}
}

void welcome()
{
	const std::string frutibandas =
	"========================================================\n"
	"   _            _   _ _                     _           \n"
	" / _|_ __ _   _| |_(_) |__   __ _ _ __   __| | __ _ ___ \n"
	"| |_| '__| | | | __| | '_ \\ / _` | '_ \\ / _` |/ _` / __|\n"
	"|  _| |  | |_| | |_| | |_) | (_| | | | | (_| | (_| \\__ \\\n"
	"|_| |_|   \\__,_|\\__|_|_.__/ \\__,_|_| |_|\\__,_|\\__,_|___/\n"
	"========================================================\n";
	std::cout << frutibandas << std::endl;
}

int main(int argc, char* argv[])
{
	// Welcome message
	welcome();

	// set server port
	if(argc == 1)
	{
		std::cout << "Server listens on port : " << 7777 << std::endl;
	}
	if(argc == 2)
	{
		std::string port_str = argv[1];
		if(isNumber(port_str)){
			enet_uint16 port = static_cast<enet_uint16>(std::atoi(port_str.c_str()));
			g_server.set_port(port);
			std::cout << "Server listens on port : " << port << std::endl;
		}
		else{
			std::cerr << "ERROR::INVALID_ARGUMENT = port is not a number, you must provide a valid number." << std::endl;
			std::exit(-1);
		}
	}

	// thread for server commands
	std::thread thr_cmd(commands);

	// pool of threads
	const unsigned int num_threads{ std::thread::hardware_concurrency() };
	std::vector<std::thread> thread_pool;
	for (unsigned int i{ 0 }; i < 2/*num_threads*/; ++i)
	{
		thread_pool.emplace_back(message_processing, i + 1);
	}

	// run server
	g_server.run();
	
	// join threads
	for (auto& thr : thread_pool) {
		thr.join();
	}
	thr_cmd.join();

	return 0;
}
