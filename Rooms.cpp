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

bool DoRoomsCollide(const Room& a, const Room& b)
{
    // Simple 2D axis-aligned bounding box collision detection
    return a.x < (b.x + b.length) &&  // asd
           (a.x + a.length) > b.x &&  // as
           a.y < (b.y + b.width) &&   // a
           (a.y + a.width) > b.y;     // asdfasd
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
            if (!FuzzyEquals(proportionLW, LIVING_PROPORTION) &&
                !FuzzyEquals(proportionWL, LIVING_PROPORTION))
                return false;
            break;
        case RoomType::KITCHEN:
            if (!KITCHEN_LENGTH.Contains(room.length)) return false;
            if (!KITCHEN_WIDTH.Contains(room.width)) return false;
            if (!KITCHEN_AREA.Contains(area)) return false;
            if (!KITCHEN_PROPORTION.Contains(proportionLW) &&
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
            if (!HALL_PROPORTION.Contains(proportionLW) && !HALL_PROPORTION.Contains(proportionWL))
                return false;
            break;
        case RoomType::BED1:
            if (!BED1_LENGTH.Contains(room.length)) return false;
            if (!BED1_WIDTH.Contains(room.width)) return false;
            if (!BED1_AREA.Contains(area)) return false;
            if (!FuzzyEquals(proportionLW, BED1_PROPORTION) &&
                !FuzzyEquals(proportionWL, BED1_PROPORTION))
                return false;
            break;
        case RoomType::BED2:
            if (!BED2_LENGTH.Contains(room.length)) return false;
            if (!BED2_WIDTH.Contains(room.width)) return false;
            if (!BED2_AREA.Contains(area)) return false;
            if (!FuzzyEquals(proportionLW, BED2_PROPORTION) &&
                !FuzzyEquals(proportionWL, BED2_PROPORTION))
                return false;
            break;
        case RoomType::BED3:
            if (!BED3_LENGTH.Contains(room.length)) return false;
            if (!BED3_WIDTH.Contains(room.width)) return false;
            if (!BED3_AREA.Contains(area)) return false;
            if (!FuzzyEquals(proportionLW, BED3_PROPORTION) &&
                !FuzzyEquals(proportionWL, BED3_PROPORTION))
                return false;
            break;
        default:
            return false;
    }

    return true;
}

RoomValidity DoesRoomFitContraintsDiganostic(const Room& room)
{
    const float area = room.length * room.width;
    const float proportionLW = room.length / room.width;
    const float proportionWL = room.width / room.length;

    RoomValidity validity;
    validity.lengthMet = true;
    validity.widthMet = true;
    validity.xMet = true;
    validity.yMet = true;
    validity.areaMet = true;
    validity.proportionMet = true;

    switch (room.type)
    {
        case RoomType::LIVING:
            if (!LIVING_LENGTH.Contains(room.length)) validity.lengthMet = false;
            if (!LIVING_WIDTH.Contains(room.width)) validity.widthMet = false;
            if (!LIVING_AREA.Contains(area)) validity.areaMet = false;
            if (!FuzzyEquals(proportionLW, LIVING_PROPORTION) &&
                !FuzzyEquals(proportionWL, LIVING_PROPORTION))
                validity.proportionMet = false;
            break;
        case RoomType::KITCHEN:
            if (!KITCHEN_LENGTH.Contains(room.length)) validity.lengthMet = false;
            if (!KITCHEN_WIDTH.Contains(room.width)) validity.widthMet = false;
            if (!KITCHEN_AREA.Contains(area)) validity.areaMet = false;
            if (!KITCHEN_PROPORTION.Contains(proportionLW) &&
                !KITCHEN_PROPORTION.Contains(proportionWL))
                validity.proportionMet = false;
            break;
        case RoomType::BATH:
            if (!FuzzyEquals(room.length, BATH_LENGTH)) validity.lengthMet = false;
            if (!FuzzyEquals(room.width, BATH_WIDTH)) validity.widthMet = false;
            break;
        case RoomType::HALL:
            if (!FuzzyEquals(room.length, HALL_LENGTH)) validity.lengthMet = false;
            if (!HALL_WIDTH.Contains(room.width)) validity.widthMet = false;
            if (!HALL_AREA.Contains(area)) validity.areaMet = false;
            if (!HALL_PROPORTION.Contains(proportionLW) && !HALL_PROPORTION.Contains(proportionWL))
                validity.proportionMet = false;
            break;
        case RoomType::BED1:
            if (!BED1_LENGTH.Contains(room.length)) validity.lengthMet = false;
            if (!BED1_WIDTH.Contains(room.width)) validity.widthMet = false;
            if (!BED1_AREA.Contains(area)) validity.areaMet = false;
            if (!FuzzyEquals(proportionLW, BED1_PROPORTION) &&
                !FuzzyEquals(proportionWL, BED1_PROPORTION))
                validity.proportionMet = false;
            break;
        case RoomType::BED2:
            if (!BED2_LENGTH.Contains(room.length)) validity.lengthMet = false;
            if (!BED2_WIDTH.Contains(room.width)) validity.widthMet = false;
            if (!BED2_AREA.Contains(area)) validity.areaMet = false;
            if (!FuzzyEquals(proportionLW, BED2_PROPORTION) &&
                !FuzzyEquals(proportionWL, BED2_PROPORTION))
                validity.proportionMet = false;
            break;
        case RoomType::BED3:
            if (!BED3_LENGTH.Contains(room.length)) validity.lengthMet = false;
            if (!BED3_WIDTH.Contains(room.width)) validity.widthMet = false;
            if (!BED3_AREA.Contains(area)) validity.areaMet = false;
            if (!FuzzyEquals(proportionLW, BED3_PROPORTION) &&
                !FuzzyEquals(proportionWL, BED3_PROPORTION))
                validity.proportionMet = false;
            break;
        default:
            return RoomValidity{};  // should be all false
    }

    return validity;
}

std::string_view RoomTypeToString(RoomType type)
{
    switch (type)
    {
        case RoomType::LIVING:
            return "Living";
        case RoomType::KITCHEN:
            return "Kitchen";
        case RoomType::BATH:
            return "Bath";
        case RoomType::HALL:
            return "Hall";
        case RoomType::BED1:
            return "Bed 1";
        case RoomType::BED2:
            return "Bed 2";
        case RoomType::BED3:
            return "Bed 3";
        default:
            return "<UNKNOWN ROOM>";
    }
}