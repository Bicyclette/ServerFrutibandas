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
#include <array>
#include <random>
#include <thread>
#include <sstream>
#include <mutex>
#include <queue>

struct Game;

// "nn"  : nickname
// "pp"  : profile picture
// "so"  : search opponent
// "sso" : stop search opponent
// "gc:" : game chat
// "mv"  : moving fruits

struct Avatar
{
	int m_gender;		// 0 : male, 1 : female
	int m_hair;			// [0,7] : [mixte, herisson, decoiffe, arriere, meche avant, mi-long, frange, au_bol, ponytail]
	int m_eyes;			// [0,4] : [manga, amande, gros, egypte, mascara]
	int m_mouth;		// [0,2] : [petite, moyenne, grande]
	int m_skin_color;	// [0,9] : [pale1, pale2, pale3, mc1, mc2, mc3, m1, m2, m3, m4]
	int m_hair_color;	// [0,12] : [rouge, orange, j1, j2, v1, v2, b1, b2, b3, pourpre, violet, rose, rose_bonbon]
	int m_eyes_color;	// [0,6] : [jaune, marron, rouge, violet, rose, bleu, vert]
	std::string m_data;	// all above data for network
};

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
	Avatar m_avatar;
	std::shared_ptr<Game> m_game;
	ENetPeer* m_peer;
	bool m_in_queue;
	bool m_in_game;
};

struct Fruit
{
	// >>>>>>>>>> methods
	Fruit() : m_type(-1), m_petrified(false) {}
	// >>>>>>>>>> properties
	int m_type;	// -1 = none, 0 = orange, 1 = banane
	bool m_petrified;
};

struct Tile
{
	// >>>>>>>>>> methods
	Tile() : m_alive(true), m_trap(false) {}

	// >>>>>>>>>> properties
	bool m_alive;
	bool m_trap;
};

struct Board
{
	Board() :
		boundLeft(0),
		boundRight(7),
		boundTop(0),
		boundBottom(7),
		m_charge(false),
		m_solo(false),
		m_solo_location(-1,-1)
	{}

	int orange_count()
	{
		int count{ 0 };
		for (int i{ 0 }; i < 8; ++i) {
			for (int j{ 0 }; j < 8; ++j) {
				count += (m_fruit[j][i].m_type == 0) ? 1 : 0;
			}
		}
		return count;
	}

	int banane_count()
	{
		int count{ 0 };
		for (int i{ 0 }; i < 8; ++i) {
			for (int j{ 0 }; j < 8; ++j) {
				count += (m_fruit[j][i].m_type == 1) ? 1 : 0;
			}
		}
		return count;
	}

	void print()
	{
		for (int line{ boundTop }; line <= boundBottom; ++line)
		{
			for (int col{ boundLeft }; col <= boundRight; ++col)
			{
				int t = m_fruit[line][col].m_type;
				if (t > -1) {
					std::cout << ' ' << m_fruit[line][col].m_type << ' ';

				}
				else {
					std::cout << m_fruit[line][col].m_type << ' ';
				}
			}
			std::cout << std::endl;
		}
	}

	void update_up(int fruit)
	{
		int enemy = (fruit == 0) ? 1 : 0;

		if (m_solo)
		{
			int x = m_solo_location[0];
			int y = m_solo_location[1];
			// find first gap
			int line;
			for (line = y; line > boundTop; --line)
			{
				if (m_fruit[line][x].m_type == -1 || m_fruit[line][x].m_type == -2) {
					break;
				}
			}
			if (m_fruit[line][x].m_type == -1 || m_fruit[line][x].m_type == -2)
			{
				line++;
			}
			for (; line <= y; ++line)
			{
				if (line - 1 >= boundTop && m_fruit[line - 1][x].m_type != -2) {
					m_fruit[line - 1][x].m_type = m_fruit[line][x].m_type;
				}
				m_fruit[line][x].m_type = -1;
			}
			m_solo = false;
			m_solo_location[0] = -1;
			m_solo_location[1] = -1;
		}
		else
		{
			for (int line{ boundTop }; line <= boundBottom; ++line)
			{
				for (int col{ boundLeft }; col <= boundRight; ++col)
				{
					if (m_fruit[line][col].m_type == enemy) {
						//check if there is a pusher down the column
						int l{ line };
						while (l <= boundBottom && m_fruit[l][col].m_type == enemy) {
							l++;
						}
						if (l == boundBottom + 1 || m_fruit[l][col].m_type == -2) {
							continue;
						}
						else if (m_fruit[l][col].m_type == fruit) {
							m_fruit[line][col].m_type = -1;
							if ((line - 1) >= boundTop && m_fruit[line - 1][col].m_type != -2) {
								m_fruit[line - 1][col].m_type = enemy;
							}
						}
					}
					else if (m_fruit[line][col].m_type == fruit) {
						m_fruit[line][col].m_type = -1;
						if ((line - 1) >= boundTop && m_fruit[line - 1][col].m_type != -2) {
							m_fruit[line - 1][col].m_type = fruit;
						}
					}
				}
			}
		}
	}

	void update_down(int fruit)
	{
		int enemy = (fruit == 0) ? 1 : 0;

		if (m_solo)
		{
			int x = m_solo_location[0];
			int y = m_solo_location[1];
			// find first gap
			int line;
			for (line = y; line < boundBottom; ++line)
			{
				if (m_fruit[line][x].m_type == -1 || m_fruit[line][x].m_type == -2) {
					break;
				}
			}
			if (m_fruit[line][x].m_type == -1 || m_fruit[line][x].m_type == -2)
			{
				line--;
			}
			for (; line >= y; --line)
			{
				if (line + 1 <= boundBottom && m_fruit[line + 1][x].m_type != -2) {
					m_fruit[line + 1][x].m_type = m_fruit[line][x].m_type;
				}
				m_fruit[line][x].m_type = -1;
			}
			m_solo = false;
			m_solo_location[0] = -1;
			m_solo_location[1] = -1;
		}
		else
		{
			for (int line{ boundBottom }; line >= boundTop; --line)
			{
				for (int col{ boundLeft }; col <= boundRight; ++col)
				{
					if (m_fruit[line][col].m_type == enemy) {
						//check if there is a pusher up the column
						int l{ line };
						while (l >= boundTop && m_fruit[l][col].m_type == enemy) {
							l--;
						}
						if (l == (boundTop - 1)) {
							continue;
						}
						else if (m_fruit[l][col].m_type == fruit) {
							m_fruit[line][col].m_type = -1;
							if ((line + 1) <= boundBottom && m_fruit[line + 1][col].m_type != -2) {
								m_fruit[line + 1][col].m_type = enemy;
							}
						}
					}
					else if (m_fruit[line][col].m_type == fruit) {
						m_fruit[line][col].m_type = -1;
						if ((line + 1) <= boundBottom && m_fruit[line + 1][col].m_type != -2) {
							m_fruit[line + 1][col].m_type = fruit;
						}
					}
				}
			}
		}
	}

	void update_right(int fruit)
	{
		int enemy = (fruit == 0) ? 1 : 0;

		if (m_solo)
		{
			int x = m_solo_location[0];
			int y = m_solo_location[1];
			// find first gap
			int col;
			for (col = x; col < boundRight; ++col)
			{
				if (m_fruit[y][col].m_type == -1 || m_fruit[y][col].m_type == -2) {
					break;
				}
			}
			if (m_fruit[y][col].m_type == -1 || m_fruit[y][col].m_type == -2)
			{
				col--;
			}
			for (; col >= x; --col)
			{
				if (col + 1 <= boundRight && m_fruit[y][col + 1].m_type != -2) {
					m_fruit[y][col + 1].m_type = m_fruit[y][col].m_type;
				}
				m_fruit[y][col].m_type = -1;
			}
			m_solo = false;
			m_solo_location[0] = -1;
			m_solo_location[1] = -1;
		}
		else
		{
			for (int line{ boundTop }; line <= boundBottom; ++line)
			{
				for (int col{ boundRight }; col >= boundLeft; --col)
				{
					if (m_fruit[line][col].m_type == enemy) {
						//check if there is a pusher on the left side of the line
						int c{ col };
						while (c >= boundLeft && m_fruit[line][c].m_type == enemy) {
							c--;
						}
						if (c == boundLeft - 1) {
							continue;
						}
						else if (m_fruit[line][c].m_type == fruit) {
							m_fruit[line][col].m_type = -1;
							if ((col + 1) <= boundRight && m_fruit[line][col + 1].m_type != -2) {
								m_fruit[line][col + 1].m_type = enemy;
							}
						}
					}
					else if (m_fruit[line][col].m_type == fruit) {
						m_fruit[line][col].m_type = -1;
						if ((col + 1) <= boundRight && m_fruit[line][col + 1].m_type != -2) {
							m_fruit[line][col + 1].m_type = fruit;
						}
					}
				}
			}
		}
	}

	void update_left(int fruit)
	{
		int enemy = (fruit == 0) ? 1 : 0;

		if (m_solo)
		{
			int x = m_solo_location[0];
			int y = m_solo_location[1];
			// find first gap
			int col;
			for (col = x; col > boundLeft; --col)
			{
				if (m_fruit[y][col].m_type == -1 || m_fruit[y][col].m_type == -2) {
					break;
				}
			}
			if (m_fruit[y][col].m_type == -1 || m_fruit[y][col].m_type == -2)
			{
				col++;
			}
			for (; col <= x; ++col)
			{
				if (col - 1 >= boundLeft && m_fruit[y][col - 1].m_type != -2) {
					m_fruit[y][col - 1].m_type = m_fruit[y][col].m_type;
				}
				m_fruit[y][col].m_type = -1;
			}
			m_solo = false;
			m_solo_location[0] = -1;
			m_solo_location[1] = -1;
		}
		else
		{
			for (int line{ boundTop }; line <= boundBottom; ++line)
			{
				for (int col{ boundLeft }; col <= boundRight; ++col)
				{
					if (m_fruit[line][col].m_type == enemy) {
						//check if there is a pusher on the right side of the line
						int c{ col };
						while (c <= boundRight && m_fruit[line][c].m_type == enemy) {
							c++;
						}
						if (c == boundRight + 1) {
							continue;
						}
						else if (m_fruit[line][c].m_type == fruit) {
							m_fruit[line][col].m_type = -1;
							if ((col - 1) >= boundLeft && m_fruit[line][col - 1].m_type != -2) {
								m_fruit[line][col - 1].m_type = enemy;
							}
						}
					}
					else if (m_fruit[line][col].m_type == fruit) {
						m_fruit[line][col].m_type = -1;
						if ((col - 1) >= boundLeft && m_fruit[line][col - 1].m_type != -2) {
							m_fruit[line][col - 1].m_type = fruit;
						}
					}
				}
			}
		}
	}

	void update_boundaries()
	{
		int min_x{ 8 };
		int max_x{ -1 };
		int min_y{ 8 };
		int max_y{ -1 };
		for (int line{ boundTop }; line <= boundBottom; ++line)
		{
			for (int col{ boundLeft }; col <= boundRight; ++col)
			{
				int t{ m_fruit[line][col].m_type };
				if (t != -1 && t != -2)
				{
					if (col < min_x) {
						min_x = col;
					}
					if (col > max_x) {
						max_x = col;
					}
					if (line > max_y) {
						max_y = line;
					}
					if (line < min_y) {
						min_y = line;
					}
				}
			}
		}
		boundLeft = min_x;
		boundRight = max_x;
		boundTop = min_y;
		boundBottom = max_y;
		std::cout << "bound Left = " << boundLeft << std::endl;
		std::cout << "bound Right = " << boundRight << std::endl;
		std::cout << "bound Top = " << boundTop << std::endl;
		std::cout << "bound Bottom = " << boundBottom << std::endl;
	}

	Tile m_tile[8][8];
	Fruit m_fruit[8][8];
	int boundLeft;
	int boundRight;
	int boundTop;
	int boundBottom;

	bool m_solo;
	glm::ivec2 m_solo_location;
	bool m_charge;
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
				if (!std::count(orange.begin(), orange.end(), pos)) {
					m_board.m_fruit[j][i].m_type = 1; // banane
				}
				else {
					m_board.m_fruit[j][i].m_type = 0; // orange
				}
			}
		}
		for (int i{ 0 }; i < 6; ++i)
		{
			int card_id;
			do
			{
				card_id = card_gen(gen);
			} while (std::count(card.begin(), card.end(), card_id) > 0);
			card[i] = card_id;
		}

		// set initial turn
		std::uniform_int_distribution<> turn_gen(0, 1);
		turn = turn_gen(gen);
		
		// winner
		winner = -1;
	}

	void update_turn()
	{
		turn_mtx.lock();
		if (turn == 0) {
			turn = 1;
		}
		else if(turn == 1) {
			turn = 0;
		}
		turn_mtx.unlock();
	}

	bool someone_won()
	{
		int oc{ m_board.orange_count() };
		int bc{ m_board.banane_count() };
		if (oc == 0) {
			winner = 1;
			return true;
		}
		else if (bc == 0) {
			winner = 0;
			return true;
		}
		else {
			return false;
		}
	}

	void use_card(int card_id, int fruit, int line = -1, int col = -1)
	{
		if (card_id == 0) // enclume
		{
			m_board.m_fruit[line][col].m_type = -2;
			m_board.m_tile[line][col].m_alive = false;
		}
		else if (card_id == 1) // c�l�rit�
		{
			update_turn();
		}
		else if (card_id == 2) // confiscation
		{
			//m_confiscation = true;
			//m_team_confiscation = fruit;
		}
		else if (card_id == 3) // renfort
		{
			// generate three fruits on empty tiles
			std::vector<glm::ivec2> free_pos;
			for (int line{ m_board.boundTop }; line <= m_board.boundBottom; ++line)
			{
				for (int col{ m_board.boundLeft }; col <= m_board.boundRight; ++col)
				{
					if (m_board.m_fruit[line][col].m_type == -1) {
						free_pos.push_back(glm::ivec2(line, col));
					}
				}
			}
			if (free_pos.size() == 0) {
				m_renfort = "0:-1:-1:-1:-1:-1:-1";
			}
			else if (free_pos.size() == 1) {
				std::string pos1_line = std::to_string(free_pos[0][0]);
				std::string pos1_col = std::to_string(free_pos[0][1]);
				m_renfort = "1:" + pos1_line + ":" + pos1_col + ":-1:-1:-1:-1";
				// update board
				m_board.m_fruit[free_pos[0][0]][free_pos[0][1]].m_type = fruit;
			}
			else if (free_pos.size() == 2) {
				std::string pos1_line = std::to_string(free_pos[0][0]);
				std::string pos1_col = std::to_string(free_pos[0][1]);
				std::string pos2_line = std::to_string(free_pos[1][0]);
				std::string pos2_col = std::to_string(free_pos[1][1]);
				m_renfort = "2:" + pos1_line + ":" + pos1_col + ":" + pos2_line + ":" + pos2_col + ":-1:-1";
				// update board
				m_board.m_fruit[free_pos[0][0]][free_pos[0][1]].m_type = fruit;
				m_board.m_fruit[free_pos[1][0]][free_pos[1][1]].m_type = fruit;
			}
			else if (free_pos.size() == 3) {
				std::string pos1_line = std::to_string(free_pos[0][0]);
				std::string pos1_col = std::to_string(free_pos[0][1]);
				std::string pos2_line = std::to_string(free_pos[1][0]);
				std::string pos2_col = std::to_string(free_pos[1][1]);
				std::string pos3_line = std::to_string(free_pos[2][0]);
				std::string pos3_col = std::to_string(free_pos[2][1]);
				m_renfort = "3:" + pos1_line + ":" + pos1_col + ":" + pos2_line + ":" + pos2_col + ":" + pos3_line + ":" + pos3_col;
				// update board
				m_board.m_fruit[free_pos[0][0]][free_pos[0][1]].m_type = fruit;
				m_board.m_fruit[free_pos[1][0]][free_pos[1][1]].m_type = fruit;
				m_board.m_fruit[free_pos[2][0]][free_pos[2][1]].m_type = fruit;
			}
			else {
				std::random_device rd;
				std::mt19937 gen(rd());
				m_renfort += "3";
				for (int i{ 0 }; i < 3; ++i) {
					std::uniform_int_distribution<> rand_gen(0, free_pos.size()-1);
					int index = rand_gen(gen);
					std::string pos_line = std::to_string(free_pos[index][0]);
					std::string pos_col = std::to_string(free_pos[index][1]);
					free_pos.erase(free_pos.begin() + index);
					m_renfort += ":" + pos_line + ":" + pos_col;
					// update board
					m_board.m_fruit[free_pos[index][0]][free_pos[index][1]].m_type = fruit;
				}
			}
		}
		else if (card_id == 4) // d�sordre
		{
			
		}
		else if (card_id == 5) // p�trification
		{
			//m_board.m_fruit[line][col].m_petrified = true;
		}
		else if (card_id == 6) // vachette
		{
			// delete all fruits on column
			//for (int l{ 0 }; l < 8; ++l)
			//{
				//m_board.m_fruit[l][col].m_type = -1;
			//}
		}
		else if (card_id == 7) // conversion
		{
			m_board.m_fruit[line][col].m_type = fruit;
		}
		else if (card_id == 8) // charge
		{
			m_board.m_charge = true;
		}
		else if (card_id == 9) // entracte
		{
			update_turn();
		}
		else if (card_id == 10) // solo
		{
			solo_mtx.lock();
			m_board.m_solo = true;
			solo_mtx.unlock();
			solo_location_mtx.lock();
			m_board.m_solo_location = glm::ivec2(col, line);
			solo_location_mtx.unlock();
		}
		else if (card_id == 11) // pi�ge
		{
			//m_board.m_tile[line][col].m_trap = true;
		}
		m_board.print();
		std::cout << "oranges = " << m_board.orange_count() << std::endl;
		std::cout << "bananes = " << m_board.banane_count() << std::endl;
	}

	std::shared_ptr<Player> m_player[2];
	Board m_board;
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
	std::array<int, 6> card = {-1,-1,-1,-1,-1,-1};
	int cardOwner[6] = { 0,0,0,1,1,1 }; // reference the index of the player in the m_player array
	int turn{-1}; // reference the index of the player in the m_player array
	float remaining_time[2] = {600.0f, 600.0f}; // in seconds (10 min)
	int winner;
	std::string m_renfort;

	// sync
	std::mutex turn_mtx;
	std::mutex winner_mtx;
	std::mutex remaining_time_mtx;
	std::mutex solo_mtx;
	std::mutex solo_location_mtx;
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

class NetworkServer
{
	public:

		NetworkServer();
		~NetworkServer();
		bool& get_active() { return m_active; }
		void run();
		void shutdown();
		void send_data(ENetPeer* peer, std::string data);

	private:

		void matchmaking();

	private:

		bool m_active;
		ENetAddress m_address;
		ENetHost* m_server;
		ENetEvent m_event;
		std::vector<std::shared_ptr<Player>> m_player;
		std::vector<std::shared_ptr<Game>> m_game;
};

inline std::queue<ClientMessage> g_message_queue;
inline std::mutex g_message_mutex;
inline std::mutex g_server_mutex;

#endif