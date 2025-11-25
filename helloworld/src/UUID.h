#pragma once
#include <nlohmann/json.hpp>
#include <cstdint> // for uint64_t
typedef std::uint64_t VroomUUID;


class UUIDGen {
public:
	UUIDGen();
	~UUIDGen();

	static VroomUUID GenerateUUID();

};
