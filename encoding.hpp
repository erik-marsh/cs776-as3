#pragma once

#include <array>
#include <cstdint>

#include "Rooms.hpp"

constexpr int NUM_ROOMS = 7;
constexpr int FLOAT_BITWIDTH = 10;
constexpr int ROOM_BITWIDTH = FLOAT_BITWIDTH * 4;
constexpr int CHROMOSOME_BITWIDTH = ROOM_BITWIDTH * NUM_ROOMS;

using Chromosome = std::array<uint8_t, CHROMOSOME_BITWIDTH>;

std::array<uint8_t, FLOAT_BITWIDTH> EncodeFloat(float val);
Chromosome EncodeChromosome(std::array<Room, NUM_ROOMS>& rooms);

float DecodeFloat(std::array<uint8_t, FLOAT_BITWIDTH>& bitstring);
std::array<Room, NUM_ROOMS> DecodeChromosome(Chromosome& chromosome);
