#pragma once

#include <string_view>

template <typename T>
struct Range
{
    constexpr Range(T low_, T high_) : low(low_), high(high_) {}

    const T low;
    const T high;

    const bool Contains(T val) const
    {
        if (val < low) return false;
        if (val > high) return false;
        return true;
    }
};

constexpr Range<float> LIVING_LENGTH(8.0f, 20.0f);
constexpr Range<float> LIVING_WIDTH(8.0f, 20.0f);
constexpr Range<float> LIVING_AREA(120.0f, 300.0f);
constexpr float LIVING_PROPORTION = 1.5f;

constexpr Range<float> KITCHEN_LENGTH(6.0f, 18.0f);
constexpr Range<float> KITCHEN_WIDTH(6.0f, 18.0f);
constexpr Range<float> KITCHEN_AREA(50.0f, 120.0f);
constexpr Range<float> KITCHEN_PROPORTION(1.0f, 1.5f);

constexpr float BATH_LENGTH = 5.5f;
constexpr float BATH_WIDTH = 8.5f;

constexpr float HALL_LENGTH = 5.5f;
constexpr Range<float> HALL_WIDTH(3.5f, 6.0f);
constexpr Range<float> HALL_AREA(19.0f, 72.0f);
constexpr Range<float> HALL_PROPORTION(1.0f, 1.5f);

constexpr Range<float> BED1_LENGTH(10.0f, 17.0f);
constexpr Range<float> BED1_WIDTH(10.0f, 17.0f);
constexpr Range<float> BED1_AREA(100.0f, 180.0f);
constexpr float BED1_PROPORTION = 1.5f;

constexpr Range<float> BED2_LENGTH(9.0f, 20.0f);
constexpr Range<float> BED2_WIDTH(9.0f, 20.0f);
constexpr Range<float> BED2_AREA(100.0f, 180.0f);
constexpr float BED2_PROPORTION = 1.5f;

constexpr Range<float> BED3_LENGTH(8.0f, 18.0f);
constexpr Range<float> BED3_WIDTH(8.0f, 18.0f);
constexpr Range<float> BED3_AREA(100.0f, 180.0f);
constexpr float BED3_PROPORTION = 1.5f;

enum class RoomType
{
    LIVING,
    KITCHEN,
    BATH,
    HALL,
    BED1,
    BED2,
    BED3
};

struct Room
{
    float length;
    float width;
    float x;
    float y;
    RoomType type;
};

struct RoomValidity
{
    bool lengthMet;
    bool widthMet;
    bool xMet;
    bool yMet;
    bool areaMet;
    bool proportionMet;
};

bool FuzzyEquals(float x, float y);
float RoomCost(const Room& room);
bool DoesRoomFitConstraints(const Room& room);
RoomValidity DoesRoomFitContraintsDiganostic(const Room& room);
std::string_view RoomTypeToString(RoomType type);