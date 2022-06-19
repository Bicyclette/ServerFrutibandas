#include "logic.hpp"

std::string Board::to_string()
{
	std::string data;
	for (int line = 0; line < 8; ++line)
	{
		for (int col = 0; col < 8; ++col)
		{
			if (tile[col][line].fruit->type == Fruit::TYPE::BANANE) {
				if (col == 7) {
					data += "b";
				}
				else {
					data += "b.";
				}
			}
			else {
				if (col == 7) {
					data += "o";
				}
				else {
					data += "o.";
				}
			}
		}
		if (line < 7) {
			data += "\n";
		}
	}
	return data;
}