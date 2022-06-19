#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>

class RandomGenerator
{
public:
	RandomGenerator(int from, int to) :
		m_random_engine(m_random_device()),
		m_distribution(from, to)
	{}

	int gen()
	{
		return m_distribution(m_random_engine);
	}

public:
	std::random_device m_random_device;
	std::mt19937 m_random_engine;
	std::uniform_int_distribution<int> m_distribution;
};

#endif