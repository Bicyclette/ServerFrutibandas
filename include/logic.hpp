#ifndef LOGIC_HPP
#define LOGIC_HPP

#include <string>
#include "random.hpp"

// Bounding Box
struct AABB
{
	int left;
	int right;
	int bottom;
	int top;
};

struct Fruit
{
	enum TYPE
	{
		BANANE,
		ORANGE
	};

	enum STATE
	{
		ALIVE,
		DEAD,
		PETRIFIED
	};

	Fruit() : state(STATE::ALIVE)
	{}

	TYPE type;
	STATE state;
};

struct Tile
{
	enum STATE
	{
		ALIVE,
		DEAD,
		TRAPPED
	};

	Tile() : state(STATE::ALIVE), left(nullptr), right(nullptr), down(nullptr), up(nullptr), fruit(nullptr)
	{}

	STATE state;
	Tile* left;
	Tile* right;
	Tile* down;
	Tile* up;
	Fruit* fruit;
};

// mouvement en vague
struct Wave
{
	Wave()
	{
		waveLeftStart = -1;
		waveRightStart = -1;
		waveUpStart = -1;
		waveDownStart = -1;
	}
	int waveLeftStart;
	int waveRightStart;
	int waveUpStart;
	int waveDownStart;
};

struct Board
{
	Tile tile[8][8];
	AABB boundingBox;
	Wave waveOrange;
	Wave waveBanane;

	std::string to_string();
};

#endif