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
			// check if the player sending a message is in game
			player->in_game_mtx.lock();
			bool in_game = player->m_in_game;
			player->in_game_mtx.unlock();
			if (!in_game) { continue; }

			std::shared_ptr<Game> game{ player->m_game };
			int who{ (player == game->m_player[0]) ? 0 : 1 };

			std::string header0("gc:");
			std::string chat;
			chat += player->m_name;
			chat += " > ";
			chat += data.substr(3);
			std::string header1(std::to_string(data.substr(3).size()) + ":");
			std::string chatMsg = header0 + header1 + chat;
			g_server_mutex.lock();
			server.send_data(game->m_player[0]->m_peer, chatMsg); // to orange
			server.send_data(game->m_player[1]->m_peer, chatMsg); // to banane
			g_server_mutex.unlock();
		}
		else if (data[0] == 'm' && data[1] == 'v') // moving fruits
		{
			// check if the player sending a message is in game
			player->in_game_mtx.lock();
			bool in_game = player->m_in_game;
			player->in_game_mtx.unlock();
			if (!in_game) { continue; }

			std::shared_ptr<Game> game{ player->m_game };
			int who{ (player == game->m_player[0]) ? 0 : 1 };

			int dir{std::atoi(&data[3])};
			if (dir == 1)
			{
				if (game->m_board.m_charge) {
					game->m_board.update_up(who);
					game->m_board.update_up(who);
					game->m_board.m_charge = false;
				}
				else {
					game->m_board.update_up(who);
				}
			}
			else if (dir == 2)
			{
				if (game->m_board.m_charge) {
					game->m_board.update_down(who);
					game->m_board.update_down(who);
					game->m_board.m_charge = false;
				}
				else {
					game->m_board.update_down(who);
				}
			}
			else if (dir == 3)
			{
				if (game->m_board.m_charge) {
					game->m_board.update_right(who);
					game->m_board.update_right(who);
					game->m_board.m_charge = false;
				}
				else {
					game->m_board.update_right(who);
				}
			}
			else if (dir == 4)
			{
				if (game->m_board.m_charge) {
					game->m_board.update_left(who);
					game->m_board.update_left(who);
					game->m_board.m_charge = false;
				}
				else {
					game->m_board.update_left(who);
				}
			}
			game->m_board.update_boundaries();
			game->m_board.print();
			std::cout << "oranges = " << game->m_board.orange_count() << std::endl;
			std::cout << "bananes = " << game->m_board.banane_count() << std::endl;
			
			std::string mv("mv:");
			mv += std::to_string(dir) + ":" + std::to_string(who);
			g_server_mutex.lock();
			server.send_data(game->m_player[0]->m_peer, mv); // to orange
			server.send_data(game->m_player[1]->m_peer, mv); // to banane
			g_server_mutex.unlock();

			if (game->someone_won()) {
				game->winner_mtx.lock();
				int game_winner = game->winner;
				game->winner_mtx.unlock();
				std::string winner = std::to_string(game_winner);
				g_server_mutex.lock();
				server.send_data(game->m_player[0]->m_peer, "win:" + winner); // to orange
				server.send_data(game->m_player[1]->m_peer, "win:" + winner); // to banane
				g_server_mutex.unlock();
			}
			else {
				game->update_turn();
				game->turn_mtx.lock();
				int turn = game->turn;
				game->turn_mtx.unlock();
				std::string next_turn("t:" + std::to_string(turn));
				g_server_mutex.lock();
				std::cout << "send next turn = " << turn << " to both players" << std::endl;
				server.send_data(game->m_player[0]->m_peer, next_turn); // to orange
				server.send_data(game->m_player[1]->m_peer, next_turn); // to banane
				g_server_mutex.unlock();
			}
		}
		else if (data[0] == 'r' && data[1] == 't')
		{
			// check if the player sending a message is in game
			player->in_game_mtx.lock();
			bool in_game = player->m_in_game;
			player->in_game_mtx.unlock();
			if (!in_game) { continue; }
			
			std::shared_ptr<Game> game{ player->m_game };
			int who{ (player == game->m_player[0]) ? 0 : 1 };

			float remaining = std::stof(data.substr(3));
			if (game->m_player[0] == player) {
				game->remaining_time[0] = remaining;
				if (remaining == 0.0f) {
					game->winner = 1;
					std::string winner = std::to_string(game->winner);
					g_server_mutex.lock();
					server.send_data(game->m_player[1]->m_peer, data); // to banane
					server.send_data(game->m_player[1]->m_peer, "win:" + winner); // to banane
					server.send_data(game->m_player[0]->m_peer, "win:" + winner); // to orange
					g_server_mutex.unlock();
				}
				else {
					g_server_mutex.lock();
					server.send_data(game->m_player[1]->m_peer, data); // to banane
					g_server_mutex.unlock();
				}
			}
			else {
				game->remaining_time[1] = remaining;
				if (remaining == 0.0f) {
					game->winner = 0;
					std::string winner = std::to_string(game->winner);
					g_server_mutex.lock();
					server.send_data(game->m_player[0]->m_peer, data); // to orange
					server.send_data(game->m_player[0]->m_peer, "win:" + winner); // to orange
					server.send_data(game->m_player[1]->m_peer, "win:" + winner); // to banane
					g_server_mutex.unlock();
				}
				else {
					g_server_mutex.lock();
					server.send_data(game->m_player[0]->m_peer, data); // to orange
					g_server_mutex.unlock();
				}
			}
		}
		else if (data[0] == 'c') // card chosen
		{
			// check if the player sending a message is in game
			player->in_game_mtx.lock();
			bool in_game = player->m_in_game;
			player->in_game_mtx.unlock();
			if (!in_game) { continue; }
			
			std::shared_ptr<Game> game{ player->m_game };
			int who{ (player == game->m_player[0]) ? 0 : 1 };
			
			std::istringstream card_stream(data);
			std::vector<int> info;
			std::string element;
			while (getline(card_stream, element, ':'))
			{
				info.push_back(std::atoi(element.c_str()));
			}
			game->use_card(info[1], who, info[2], info[3]);

			// notify enemy of the played card
			int to = (who == 0) ? 1 : 0;
			int card_id = info[1];
			if (card_id == 0) // enclume
			{
				g_server_mutex.lock();
				server.send_data(game->m_player[to]->m_peer, data);
				g_server_mutex.unlock();
			}
			else if (card_id == 1) // célérité
			{
				g_server_mutex.lock();
				server.send_data(game->m_player[to]->m_peer, data);
				g_server_mutex.unlock();
			}
			else if (card_id == 2) // confiscation
			{
				
			}
			else if (card_id == 3) // renfort
			{
				std::string renfort = data + ":" + game->m_renfort + ":" + std::to_string(who);
				game->m_renfort.clear();
				g_server_mutex.lock();
				server.send_data(game->m_player[who]->m_peer, renfort);
				server.send_data(game->m_player[to]->m_peer, renfort);
				g_server_mutex.unlock();
			}
			else if (card_id == 4) // désordre
			{
				g_server_mutex.lock();
				server.send_data(game->m_player[to]->m_peer, data);
				g_server_mutex.unlock();
			}
			else if (card_id == 5) // pétrification
			{
				
			}
			else if (card_id == 6) // vachette
			{
				
			}
			else if (card_id == 7) // conversion
			{
				g_server_mutex.lock();
				server.send_data(game->m_player[to]->m_peer, data);
				g_server_mutex.unlock();
			}
			else if (card_id == 8) // charge
			{
				g_server_mutex.lock();
				server.send_data(game->m_player[to]->m_peer, data);
				g_server_mutex.unlock();
			}
			else if (card_id == 9) // entracte
			{
				g_server_mutex.lock();
				server.send_data(game->m_player[to]->m_peer, data);
				game->turn_mtx.lock();
				int turn = game->turn;
				game->turn_mtx.unlock();
				std::string next_turn("t:" + std::to_string(turn));
				server.send_data(game->m_player[0]->m_peer, next_turn); // to orange
				server.send_data(game->m_player[1]->m_peer, next_turn); // to banane
				g_server_mutex.unlock();
			}
			else if (card_id == 10) // solo
			{
				g_server_mutex.lock();
				server.send_data(game->m_player[to]->m_peer, data);
				g_server_mutex.unlock();
			}
			else if (card_id == 11) // piège
			{
				g_server_mutex.lock();
				server.send_data(game->m_player[to]->m_peer, data);
				g_server_mutex.unlock();
			}
		}
		else if (data[0] == 'g' && data[1] == 'u') // give up
		{
			// check if the player sending a message is in game
			player->in_game_mtx.lock();
			bool in_game = player->m_in_game;
			player->in_game_mtx.unlock();
			if (!in_game) { continue; }

			std::shared_ptr<Game> game{ player->m_game };
			int who{ (player == game->m_player[0]) ? 0 : 1 };
			int to = (who == 0) ? 1 : 0;
			std::string gu_data = "gu:" + game->m_player[who]->m_name;
			
			// notify opponent
			g_server_mutex.lock();
			server.send_data(game->m_player[to]->m_peer, gu_data);
			g_server_mutex.unlock();
		}
		else if (data[0] == 'l' && data[1] == 'e' && data[2] == 'a' && data[3] == 'v' && data[4] == 'e') // a player gave up the game, and the other one left the game in consequence
		{
		std::cout << "check if leaving player is in game" << std::endl;
			// check if the player sending a message is in game
			player->in_game_mtx.lock();
			bool in_game = player->m_in_game;
			player->in_game_mtx.unlock();
			if (!in_game) { continue; }
			std::cout << "check if leaving player is in game (ALL GOOD)" << std::endl;

			
			// both players are no longer in game
			std::shared_ptr<Game> game{ player->m_game };
			game->m_player[0]->in_game_mtx.lock();
			game->m_player[0]->m_in_game = false;
			game->m_player[0]->in_game_mtx.unlock();
			game->m_player[1]->in_game_mtx.lock();
			game->m_player[1]->m_in_game = false;
			game->m_player[1]->in_game_mtx.unlock();

			// delete game instance
			g_server_mutex.lock();
			int num_games = server.m_game.size();
			for (int i = 0; i < num_games; ++i)
			{
				std::shared_ptr<Game> g{ server.m_game[i] };
				if (game == g) {
					server.m_game.erase(server.m_game.begin() + i);
					std::cout << "game instance erased" << std::endl;
					break;
				}
			}
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