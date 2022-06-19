#include "game.hpp"

Game::Game(std::shared_ptr<Player> playerA, std::shared_ptr<Player> playerB) :
	m_cards{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
	m_orange_cards{-1, -1, -1},
	m_banana_cards{-1, -1, -1}
{
	m_player_orange = playerA;
	m_player_banana = playerB;

	// set initial turn
	RandomGenerator randGen(0, 1);
	m_state.turn.type = static_cast<Fruit::TYPE>(randGen.gen());

	// board bounding box
	m_board.boundingBox.bottom = 7;
	m_board.boundingBox.top = 0;
	m_board.boundingBox.left = 0;
	m_board.boundingBox.right = 7;

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
		m_board.tile[line][col].fruit = new Fruit();
		m_board.tile[line][col].fruit->type = Fruit::TYPE::ORANGE;
	}
	for (int line = 0; line < 8; ++line)
	{
		for (int col = 0; col < 8; ++col)
		{
			if (m_board.tile[line][col].fruit == nullptr)
			{
				m_board.tile[line][col].fruit = new Fruit();
				m_board.tile[line][col].fruit->type = Fruit::TYPE::BANANE;
			}
		}
	}

	// init tiles neighbors
	for (int line{0}; line < 8; ++line)
	{
		for (int col{0}; col < 8; ++col)
		{
			m_board.tile[line][col].left = ((col - 1) >= 0) ? &(m_board.tile[line][col-1]) : nullptr;
			m_board.tile[line][col].right = ((col + 1) <= 7) ? &(m_board.tile[line][col+1]) : nullptr;
			m_board.tile[line][col].down = ((line + 1) <= 7) ? &(m_board.tile[line+1][col]) : nullptr;
			m_board.tile[line][col].up = ((line - 1) >= 0) ? &(m_board.tile[line-1][col]) : nullptr;
		}
	}

	// set wave origin
	for (int line = 0; line < 8; ++line)
	{
		for (int col = 0; col < 8; ++col)
		{
			// down & up
			if (m_board.tile[col][line].fruit->type == Fruit::TYPE::BANANE && m_board.waveBanane.waveDownStart == -1) { m_board.waveBanane.waveDownStart = line; }
			else if(m_board.tile[col][line].fruit->type == Fruit::TYPE::BANANE) { m_board.waveBanane.waveUpStart = line; }

			if (m_board.tile[col][line].fruit->type == Fruit::TYPE::ORANGE && m_board.waveOrange.waveDownStart == -1) { m_board.waveOrange.waveDownStart = line; }
			else if (m_board.tile[col][line].fruit->type == Fruit::TYPE::ORANGE) { m_board.waveOrange.waveUpStart = line; }
		}
	}
	for (int col = 0; col < 8; ++col)
	{
		for (int line = 0; line < 8; ++line)
		{
			// left & right
			if (m_board.tile[col][line].fruit->type == Fruit::TYPE::BANANE && m_board.waveBanane.waveRightStart == -1) { m_board.waveBanane.waveRightStart = col; }
			else if (m_board.tile[col][line].fruit->type == Fruit::TYPE::BANANE) { m_board.waveBanane.waveLeftStart = col; }

			if (m_board.tile[col][line].fruit->type == Fruit::TYPE::ORANGE && m_board.waveOrange.waveRightStart == -1) { m_board.waveOrange.waveRightStart = col; }
			else if (m_board.tile[col][line].fruit->type == Fruit::TYPE::ORANGE) { m_board.waveOrange.waveLeftStart = col; }
		}
	}

	// shuffle set of cards
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(m_cards.begin(), m_cards.end(), g);
}

std::string Game::get_free_cards()
{
	std::string data;
	for (int i = 0; i < 6; ++i)
	{
		auto cardO = std::find(m_orange_cards.begin(), m_orange_cards.end(), m_cards[i]);
		auto cardB = std::find(m_banana_cards.begin(), m_banana_cards.end(), m_cards[i]);
		if (cardO == std::end(m_orange_cards) && cardB == std::end(m_banana_cards))
		{
			data += std::to_string(m_cards[i].getType()) + ".";
		}
	}
	if (!data.empty()) {
		data.pop_back();
	}
	return data;
}