#include "logic.hpp"

std::string Board::to_string()
{
	std::string data;
	for (int line = bounds.top; line <= bounds.bottom; ++line)
	{
		for (int col = bounds.left; col <= bounds.right; ++col)
		{
			if (tile[col][line].fruit.type != 'x')
			{
				if (tile[col][line].fruit.type == 'b') {
					if (col == bounds.right) {
						data += "b";
					}
					else {
						data += "b.";
					}
				}
				else {
					if (col == bounds.right) {
						data += "o";
					}
					else {
						data += "o.";
					}
				}
			}
			else
			{
				if (col == bounds.right) {
					data += "x";
				}
				else {
					data += "x.";
				}
			}
		}
		if (line < bounds.bottom) {
			data += "\n";
		}
	}
	return data;
}