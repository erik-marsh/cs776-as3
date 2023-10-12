#include "encoding.hpp"

#include <cmath>
#include <cstdio>
#include <iomanip>

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
        auto xEncoded = EncodeFloat(rooms[i].x);  // vestigial, kept so encodings don't screw up
        auto yEncoded = EncodeFloat(rooms[i].y);  // vestigial, kept so encodings don't screw up

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
        room.x = roomVals[2];  // vestigial, kept so encodings don't screw up
        room.y = roomVals[3];  // vestigial, kept so encodings don't screw up
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

void PrintChromosome(Chromosome& chromosome, std::ostream& stream)
{
    static constexpr std::array<RoomType, NUM_ROOMS> roomTypes = {
        RoomType::LIVING, RoomType::KITCHEN, RoomType::BATH, RoomType::HALL,
        RoomType::BED1,   RoomType::BED2,    RoomType::BED3};

    stream << "            Length     | Width      | x Pos      | y Pos      | Type\n";
    stream << "Chromosome: ";
    for (int i = 0; i < NUM_ROOMS; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < FLOAT_BITWIDTH; k++)
            {
                int index = (i * ROOM_BITWIDTH) + (j * FLOAT_BITWIDTH) + k;
                stream << static_cast<int>(chromosome[index]);
            }
            stream << " | ";
        }
        stream << RoomTypeToString(roomTypes[i]) << "\n";

        if (i != NUM_ROOMS - 1) stream << "            ";
    }
}

void PrintRoomSet(RoomSet& rooms, std::ostream& stream)
{
    constexpr char invalidMarker = '~';

    stream << "            Length     | Width      | x Pos      | y Pos      | Area         | "
           << "PropLW     | PropWL     | Type\n";
    stream << "RoomSet...: ";
    for (int i = 0; i < NUM_ROOMS; i++)
    {
        Room& room = rooms[i];
        RoomValidity validity = DoesRoomFitContraintsDiganostic(room);

        const float area = room.length * room.width;
        const float proportionLW = room.width > 0.0f ? room.length / room.width : 0.0f;
        const float proportionWL = room.length > 0.0f ? room.width / room.length : 0.0f;

        char marker = (validity.lengthMet ? ' ' : invalidMarker);
        stream << marker << std::setfill(marker) << std::fixed << std::setw(9)
               << std::setprecision(6) << room.length << marker << '|';
        marker = (validity.widthMet ? ' ' : invalidMarker);
        stream << marker << std::setfill(marker) << std::fixed << std::setw(10)
               << std::setprecision(6) << room.width << marker << '|';
        marker = (validity.xMet ? ' ' : invalidMarker);
        stream << marker << std::setfill(marker) << std::fixed << std::setw(10)
               << std::setprecision(6) << room.x << marker << '|';  // vestigial
        marker = (validity.yMet ? ' ' : invalidMarker);
        stream << marker << std::setfill(marker) << std::fixed << std::setw(10)
               << std::setprecision(6) << room.y << marker << '|';  // vestigial
        marker = (validity.areaMet ? ' ' : invalidMarker);
        stream << marker << std::setfill(marker) << std::fixed << std::setw(12)
               << std::setprecision(6) << area << marker << '|';
        marker = (validity.proportionMet ? ' ' : invalidMarker);
        stream << marker << std::setfill(marker) << std::fixed << std::setw(10)
               << std::setprecision(6) << proportionLW << marker << '|';
        marker = (validity.proportionMet ? ' ' : invalidMarker);
        stream << marker << std::setfill(marker) << std::fixed << std::setw(10)
               << std::setprecision(6) << proportionWL << marker << '|';
        stream << ' ' << RoomTypeToString(room.type) << "\n";

        if (i != NUM_ROOMS - 1) stream << "            ";
    }
}
