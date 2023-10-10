#include "encoding.hpp"

#include <cmath>
#include <cstdio>

// The float encoder transforms a value in the range [0, 102.3]
// into a bitsting of width 10.
// We then expand on this to encode a whole chromosome (7 Rooms of 4 floats each = 280 bits).
// Living, Kitchen, Bath, Hall, Bed1, Bed2, Bed3 is the assumed order.

Gene EncodeFloat(float val)
{
    Gene ret;
    val *= 10.0f;
    int ival = static_cast<int>(std::round(val));
    for (int i = FLOAT_BITWIDTH - 1; i >= 0; i--)
    {
        ret[i] = ival % 2;
        ival /= 2;
    }
    return ret;
}

Chromosome EncodeChromosome(RoomSet& rooms)
{
    Chromosome ret;

    for (int i = 0; i < rooms.size(); i++)
    {
        auto lengthEncoded = EncodeFloat(rooms[i].length);
        auto widthEncoded = EncodeFloat(rooms[i].width);
        auto xEncoded = EncodeFloat(rooms[i].x);
        auto yEncoded = EncodeFloat(rooms[i].y);

        std::array iter = {lengthEncoded, widthEncoded, xEncoded, yEncoded};
        for (int j = 0; j < iter.size(); j++)
            for (int k = 0; k < iter[j].size(); k++)
                //   i * 40             +  j * 10              + k
                ret[(i * ROOM_BITWIDTH) + (j * FLOAT_BITWIDTH) + k] = iter[j][k];
    }

    return ret;
}

float DecodeFloat(Gene& bitstring)
{
    int ival = 0;
    for (int i = 0; i < bitstring.size(); i++)
        ival += static_cast<int>(bitstring[i]) << (bitstring.size() - i - 1);
    float val = static_cast<float>(ival) * 0.1;
    return val;
}

RoomSet DecodeChromosome(Chromosome& chromosome)
{
    static constexpr std::array<RoomType, NUM_ROOMS> roomTypes = {
        RoomType::LIVING, RoomType::KITCHEN, RoomType::BATH, RoomType::HALL,
        RoomType::BED1,   RoomType::BED2,    RoomType::BED3};

    RoomSet ret;
    for (int i = 0; i < NUM_ROOMS; i++)
    {
        Room room;
        std::array<float, 4> roomVals;

        for (int j = 0; j < 4; j++)
        {
            Gene encoded;
            for (int k = 0; k < FLOAT_BITWIDTH; k++)
                encoded[k] = chromosome[(i * ROOM_BITWIDTH) + (j * FLOAT_BITWIDTH) + k];
            float decoded = DecodeFloat(encoded);
            roomVals[j] = decoded;
        }

        room.length = roomVals[0];
        room.width = roomVals[1];
        room.x = roomVals[2];
        room.y = roomVals[3];
        room.type = roomTypes[i];
        ret[i] = room;
    }

    return ret;
}

// This function assumes that all the rooms are valid beforehand.
// (i.e. DoesRoomFitConstraints returns true for all rooms.)
float ObjectiveFunction(RoomSet& rooms)
{
    float ret = 0.0f;
    for (Room& room : rooms)
        ret += RoomCost(room);
    return ret;
}

float ObjectiveToFitness(float objectiveValue)
{
    // We have reliable minimums and maximums for the room cost
    constexpr float maxCost = 1245.5f;
    constexpr float minCost = 632.5f;
    constexpr float range = maxCost - minCost;
    // hence we can just do a simple C - f(x) formula,
    // scaled to the range [0, 100]
    return ((maxCost - objectiveValue) / range) * 100.0f;
}

void PrintChromosome(Chromosome& chromosome)
{
    static constexpr std::array<RoomType, NUM_ROOMS> roomTypes = {
        RoomType::LIVING, RoomType::KITCHEN, RoomType::BATH, RoomType::HALL,
        RoomType::BED1,   RoomType::BED2,    RoomType::BED3};

    std::printf("            %-10s | %-10s | %-10s | %-10s | %s\n", "Length", "Width", "x Pos",
                "y Pos", "Type");
    std::printf("Chromosome: ");
    for (int i = 0; i < NUM_ROOMS; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < FLOAT_BITWIDTH; k++)
            {
                int index = (i * ROOM_BITWIDTH) + (j * FLOAT_BITWIDTH) + k;
                std::printf("%d", static_cast<int>(chromosome[index]));
            }
            std::printf(" | ");
        }
        std::printf("%s\n", RoomTypeToString(roomTypes[i]).data());

        if (i != NUM_ROOMS - 1) std::printf("            ");
    }
}

void PrintRoomSet(RoomSet& rooms)
{
    static constexpr std::string_view redText = "\033[91m";
    static constexpr std::string_view defaultText = "\033[39m";
    static constexpr std::string_view resetText = "\033[0m";

    std::printf("            %-10s | %-10s | %-10s | %-10s | %-12s | %-10s | %-10s | %s\n",
                "Length", "Width", "x Pos", "y Pos", "Area", "PropLW", "PropWL", "Type");
    std::printf("RoomSet...: ");
    for (int i = 0; i < NUM_ROOMS; i++)
    {
        Room& room = rooms[i];
        RoomValidity validity = DoesRoomFitContraintsDiganostic(room);

        const float area = room.length * room.width;
        const float proportionLW =
            room.length / room.width;  // TODO: must not allow a room to be generated with 0 width
                                       // or length to prevent a division by 0
        const float proportionWL = room.width / room.length;

        std::printf("%s%10.6f%s | ", (validity.lengthMet ? defaultText.data() : redText.data()),
                    room.length, resetText.data());
        std::printf("%s%10.6f%s | ", (validity.widthMet ? defaultText.data() : redText.data()),
                    room.width, resetText.data());
        std::printf("%s%10.6f%s | ", (validity.xMet ? defaultText.data() : redText.data()), room.x,
                    resetText.data());
        std::printf("%s%10.6f%s | ", (validity.yMet ? defaultText.data() : redText.data()), room.y,
                    resetText.data());
        std::printf("%s%12.6f%s | ", (validity.areaMet ? defaultText.data() : redText.data()), area,
                    resetText.data());
        std::printf("%s%10.6f%s | ", (validity.proportionMet ? defaultText.data() : redText.data()),
                    proportionLW, resetText.data());
        std::printf("%s%10.6f%s | ", (validity.proportionMet ? defaultText.data() : redText.data()),
                    proportionWL, resetText.data());
        std::printf("%s\n", RoomTypeToString(room.type).data());

        if (i != NUM_ROOMS - 1) std::printf("            ");
    }
}
