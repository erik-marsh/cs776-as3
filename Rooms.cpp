#include "Rooms.hpp"

// Fuzzy float equality with epsilon = 0.01f;
bool FuzzyEquals(float x, float y)
{
    constexpr float epsilon = 0.01f;
    if (x < y - epsilon) return false;
    if (x > y + epsilon) return false;
    return true;
}

float RoomCost(const Room& room)
{
    const float area = room.length * room.width;
    if (room.type == RoomType::KITCHEN || room.type == RoomType::BATH) return 2.0f * area;
    return area;
}

// TODO: float equality will probably need fuzzy equality,
// since the values produced by the decoder are likely not the same as the literals.
bool DoesRoomFitConstraints(const Room& room)
{
    const float area = room.length * room.width;
    const float proportionLW = room.length / room.width;
    const float proportionWL = room.width / room.length;

    switch (room.type)
    {
        case RoomType::LIVING:
            if (!LIVING_LENGTH.Contains(room.length)) return false;
            if (!LIVING_WIDTH.Contains(room.width)) return false;
            if (!LIVING_AREA.Contains(area)) return false;
            if (!FuzzyEquals(proportionLW, LIVING_PROPORTION) ||
                !FuzzyEquals(proportionWL, LIVING_PROPORTION))
                return false;
            break;
        case RoomType::KITCHEN:
            if (!KITCHEN_LENGTH.Contains(room.length)) return false;
            if (!KITCHEN_WIDTH.Contains(room.width)) return false;
            if (!KITCHEN_AREA.Contains(area)) return false;
            if (!KITCHEN_PROPORTION.Contains(proportionLW) ||
                !KITCHEN_PROPORTION.Contains(proportionWL))
                return false;
            break;
        case RoomType::BATH:
            if (!FuzzyEquals(room.length, BATH_LENGTH)) return false;
            if (!FuzzyEquals(room.width, BATH_WIDTH)) return false;
            break;
        case RoomType::HALL:
            if (!FuzzyEquals(room.length, HALL_LENGTH)) return false;
            if (!HALL_WIDTH.Contains(room.width)) return false;
            if (!HALL_AREA.Contains(area)) return false;
            if (!HALL_PROPORTION.Contains(proportionLW) || !HALL_PROPORTION.Contains(proportionWL))
                return false;
            break;
        case RoomType::BED1:
            if (!BED1_LENGTH.Contains(room.length)) return false;
            if (!BED1_WIDTH.Contains(room.width)) return false;
            if (!BED1_AREA.Contains(area)) return false;
            if (!FuzzyEquals(proportionLW, BED1_PROPORTION) ||
                !FuzzyEquals(proportionWL, BED1_PROPORTION))
                return false;
            break;
        case RoomType::BED2:
            if (!BED2_LENGTH.Contains(room.length)) return false;
            if (!BED2_WIDTH.Contains(room.width)) return false;
            if (!BED2_AREA.Contains(area)) return false;
            if (!FuzzyEquals(proportionLW, BED2_PROPORTION) ||
                !FuzzyEquals(proportionWL, BED2_PROPORTION))
                return false;
            break;
        case RoomType::BED3:
            if (!BED3_LENGTH.Contains(room.length)) return false;
            if (!BED3_WIDTH.Contains(room.width)) return false;
            if (!BED3_AREA.Contains(area)) return false;
            if (!FuzzyEquals(proportionLW, BED3_PROPORTION) ||
                !FuzzyEquals(proportionWL, BED3_PROPORTION))
                return false;
            break;
        default:
            return false;
    }

    return true;
}