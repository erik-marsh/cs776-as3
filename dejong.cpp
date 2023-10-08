#include "dejong.hpp"

#include <array>
#include <cmath>
#include <random>

// NOTE: This only works if the range of the function contains no negative values.
// In our case, this is true for all of the De Jong functions (given their domains).
// I chose to make this a 0-100 scale for consistency with the previous assignments.
double MakeFitness(double dejongValue) { return 100.0 / (dejongValue + 1); }

double GaussianNoise()
{
    static constexpr double mean = 0.0;
    static constexpr double stddev = 1.0;

    static std::random_device device{};  // TODO: seed needs to be logged somewhere
    static std::mt19937 gen{device()};
    static std::normal_distribution<double> dist{mean, stddev};

    return dist(gen);
}

// domain is x = [-5.12, 5.12]
// minimum is f(x) = 0 for x = <0, 0, ..., 0>
double DeJong1(std::vector<double> &x)
{
    double ret = 0.0;
    for (double val : x)
        ret += val * val;
    return ret;
}

// domain is x = [-2.048, 2.048]
// minimum is f(x) = 0 for x = <1, 1, ..., 1>
double DeJong2(std::vector<double> &x)
{
    double ret = 0.0;
    for (int i = 0; i < x.size() - 1; i++)
    {
        // why were the subtractions backwards?
        double tmp1 = x[i + 1] - (x[i] * x[i]);     // minor correction
        double tmp2 = (x[i] - 1.0) * (x[i] - 1.0);  // another minor correction
        ret += (100 * tmp1 * tmp1) + tmp2;
    }
    return ret;
}

// domain is x = [-5.12, 5.12]
// minimum is f(x) = 0 for x = <[-5.12, -5), [-5.12, -5), ..., [-5.12, -5)>
double DeJong3(std::vector<double> &x)
{
    double ret = 6.0 * x.size();
    for (double val : x)
        ret += std::floor(val);
    return ret;
}

// domain is x = [-1.28, 1.28]
// minimum is indeterminate with noise,
// but without noise it is roughly f(x) = 0 for x = <0, 0, ..., 0>
double DeJong4(std::vector<double> &x)
{
    double ret = 0.0;
    for (int i = 0; i < x.size(); i++)
        ret += i * x[i] * x[i] * x[i] * x[i] + GaussianNoise();
    return ret;
}

// domain is x = [-65.536, 65.536]
// minimum is f(x) ~ 1 for x = <-32, -32>
double DeJong5(std::vector<double> &x)
{
    static constexpr std::array<std::array<int, 25>, 2> A = {
        {{-32, -16, 0,   16,  32, -32, -16, 0,   16,  32, -32, -16, 0,
          16,  32,  -32, -16, 0,  16,  32,  -32, -16, 0,  16,  32},
         {-32, -32, -32, -32, -32, -16, -16, -16, -16, -16, 0,  0, 0,
          0,   0,   16,  16,  16,  16,  16,  32,  32,  32,  32, 32}}};
    constexpr double K = 500.0;
    // c_j = j

    double ret = 1.0 / K;
    for (int j = 0; j < 25; j++)
    {
        double val = j + 1;
        for (int i = 0; i < x.size(); i++)
        {
            double diff = x[i] - A[i][j];
            double diffPow6 = diff * diff * diff * diff * diff * diff;
            val += diffPow6;
        }
        ret += 1.0 / val;
    }
    return 1.0 / ret;
}