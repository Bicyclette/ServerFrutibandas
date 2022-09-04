#ifndef LOGIC_HPP
#define LOGIC_HPP

#include <string>
#include "random.hpp"

struct Fruit
{
	enum STATE
	{
		STAND_STILL,
		MOVING,
		PETRIFIED
	};

	Fruit() : type('x'), state(STATE::STAND_STILL) {}
	Fruit(char t) : type(t), state(STATE::STAND_STILL) {}

	char type;
	STATE state;
};

struct Tile
{
	enum STATE
	{
		ALIVE,
		DEAD,
		DYING,
		TRAPPED
	};

	Tile() : state(STATE::ALIVE)
	{}

	STATE state;
	Fruit fruit;
};

struct Board
{
	struct AABB
	{
		int left;
		int right;
		int bottom;
		int top;
	};

	Tile tile[8][8];
	AABB bounds;

	Board(){}
	std::string to_string();
};

#endif