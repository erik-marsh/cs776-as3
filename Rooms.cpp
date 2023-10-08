#include "Rooms.hpp"

float RoomCost(const Room& room)
{
    const float area = room.length * room.width;
    if (room.type == RoomType::KITCHEN || room.type == RoomType::BATH) return 2.0f * area;
    return area;
}

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
            if (proportionLW != LIVING_PROPORTION || proportionWL != LIVING_PROPORTION)
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
            if (room.length != BATH_LENGTH) return false;
            if (room.width != BATH_WIDTH) return false;
            break;
        case RoomType::HALL:
            if (room.length != HALL_LENGTH) return false;
            if (!HALL_WIDTH.Contains(room.width)) return false;
            if (!HALL_AREA.Contains(area)) return false;
            if (!HALL_PROPORTION.Contains(proportionLW) || !HALL_PROPORTION.Contains(proportionWL))
                return false;
            break;
        case RoomType::BED1:
            if (!BED1_LENGTH.Contains(room.length)) return false;
            if (!BED1_WIDTH.Contains(room.width)) return false;
            if (!BED1_AREA.Contains(area)) return false;
            if (proportionLW != BED1_PROPORTION || proportionWL != BED1_PROPORTION) return false;
            break;
        case RoomType::BED2:
            if (!BED2_LENGTH.Contains(room.length)) return false;
            if (!BED2_WIDTH.Contains(room.width)) return false;
            if (!BED2_AREA.Contains(area)) return false;
            if (proportionLW != BED2_PROPORTION || proportionWL != BED2_PROPORTION) return false;
            break;
        case RoomType::BED3:
            if (!BED3_LENGTH.Contains(room.length)) return false;
            if (!BED3_WIDTH.Contains(room.width)) return false;
            if (!BED3_AREA.Contains(area)) return false;
            if (proportionLW != BED3_PROPORTION || proportionWL != BED3_PROPORTION) return false;
            break;
        default:
            return false;
    }

    return true;
}