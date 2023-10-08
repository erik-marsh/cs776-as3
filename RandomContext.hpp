#pragma once

#include <random>

struct RandomContext
{
    std::random_device::result_type seed;
    std::mt19937 gen;
};