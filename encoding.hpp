#pragma once

#include <array>
#include <cstdint>
#include <ostream>

#include "Rooms.hpp"

constexpr int NUM_ROOMS = 7;
constexpr int FLOAT_BITWIDTH = 10;
constexpr int ROOM_BITWIDTH = FLOAT_BITWIDTH * 4;
constexpr int CHROMOSOME_BITWIDTH = ROOM_BITWIDTH * NUM_ROOMS;

// objective function values to use if a certain room is invalid
// unfortunately, doing it this way means that all bathrooms are valid
constexpr std::array INVALID_OBJECTIVE = {
    LIVING_AREA.high, KITCHEN_AREA.high, BATH_WIDTH * BATH_LENGTH, HALL_AREA.high,
    BED1_AREA.high,   BED2_AREA.high,    BED3_AREA.high};

using RoomSet = std::array<Room, NUM_ROOMS>;
using Gene = std::array<uint8_t, FLOAT_BITWIDTH>;
using Chromosome = std::array<uint8_t, CHROMOSOME_BITWIDTH>;

Gene EncodeFloat(float val);
Chromosome EncodeChromosome(const RoomSet& rooms);

float DecodeFloat(const Gene& bitstring);
RoomSet DecodeChromosome(const Chromosome& chromosome);

float ObjectiveFunction(const RoomSet& rooms);
float ObjectiveToFitness(float objectiveValue);

void PrintChromosome(const Chromosome& chromosome, std::ostream& stream);
void PrintRoomSet(const RoomSet& rooms, std::ostream& stream);
