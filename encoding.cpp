#include "encoding.hpp"

#include <cmath>

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

float ObjectiveToFitness(float objectiveValue) { return 1.0f / (objectiveValue + 1.0f); }
