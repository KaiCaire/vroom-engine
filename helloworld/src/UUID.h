#pragma once
#pragma once

#include <random>
#include <pcg_random.hpp> //header only! no config required

class UUID {
public:
	UUID();
	~UUID();

	static uint64_t GenerateUUID();

};
