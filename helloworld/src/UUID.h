#pragma once

#include <cstdint> // for uint64_t

class UUIDGen {
public:
	UUIDGen();
	~UUIDGen();

	static uint64_t GenerateUUID();

};
