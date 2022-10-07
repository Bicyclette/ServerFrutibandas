#include "game.hpp"

Game::Game(std::shared_ptr<Player> playerA, std::shared_ptr<Player> playerB) :
	m_orange_cards{-1, -1, -1},
	m_banana_cards{-1, -1, -1}
{
	m_player_orange = playerA;
	m_player_banana = playerB;

	// set initial turn
	RandomGenerator randGen(0, 1);
	int turn = randGen.gen();
	m_state.turn = (turn == 0) ? 'o' : 'b';

	// board bounding box
	m_board.bounds.bottom = 7;
	m_board.bounds.top = 0;
	m_board.bounds.left = 0;
	m_board.bounds.right = 7;

	// board tiles
	RandomGenerator fruitGen(0, 7);
	std::array<int, 2> fruitPos;
	std::vector<std::array<int, 2>> oranges;
	for (int i{ 0 }; i < 32; ++i)
	{
		do
		{
			fruitPos = { fruitGen.gen(), fruitGen.gen() };
		} while (std::count(oranges.begin(), oranges.end(), fruitPos));
		oranges.push_back(fruitPos);
		int line = fruitPos[0];
		int col = fruitPos[1];
		m_board.tile[line][col].fruit.type = 'o';
	}
	for (int line = 0; line < 8; ++line)
	{
		for (int col = 0; col < 8; ++col)
		{
			if (m_board.tile[line][col].fruit.type == 'x')
			{
				m_board.tile[line][col].fruit.type = 'b';
			}
		}
	}

	// gen set of cards
	RandomGenerator cardGen(0, 11);
	std::array<int, 6> card_id = {-1,-1,-1,-1,-1,-1};
	int id = cardGen.gen();
	card_id[0] = id;
	id = cardGen.gen();
	for (int i = 1; i < 6; ++i)
	{
		while (std::find(card_id.begin(), card_id.end(), id) != card_id.end())
		{
			id = cardGen.gen();
		}
		card_id[i] = id;
	}
	m_orange_cards[0] = card_id[0];
	m_orange_cards[1] = card_id[1];
	m_orange_cards[2] = card_id[2];
	m_banana_cards[0] = card_id[3];
	m_banana_cards[1] = card_id[4];
	m_banana_cards[2] = card_id[5];
}

std::string Game::cards_to_string()
{
	std::string data;
	data += std::to_string(m_orange_cards[0].type) + ".";
	data += std::to_string(m_orange_cards[1].type) + ".";
	data += std::to_string(m_orange_cards[2].type) + ".";
	data += std::to_string(m_banana_cards[0].type) + ".";
	data += std::to_string(m_banana_cards[1].type) + ".";
	data += std::to_string(m_banana_cards[2].type);
	return data;
}