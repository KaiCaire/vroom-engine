#include "UUID.h"
#include <pcg_random.hpp> //header only library! no config required
#include <random>

UUID::UUID() {
    
}

UUID::~UUID() {

}

uint64_t UUID::GenerateUUID() {
    static pcg_extras::seed_seq_from<std::random_device> seed_source;
    static pcg32 rng(seed_source);
    static std::uniform_int_distribution<uint64_t> distribution(1, UINT64_MAX);
    return distribution(rng);
}