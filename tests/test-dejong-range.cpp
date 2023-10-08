#include <iomanip>
#include <iostream>
#include <limits>

#include "../dejong.hpp"

double FindRangeMax(double min, double max, double step,
                    double (*dejongFunc)(std::vector<double>&));
void DoTrial(double min, double max, double step, double (*dejongFunc)(std::vector<double>&));
void PrintTrial(std::vector<double>& vec, double res);

int main()
{
    // remember: dimensionality = 2
    std::cout << std::fixed << std::setprecision(8);

    // uncomment as needed or else you end up with a 10+ GB file
    // std::cout << " ========== DeJong1 ==========\n";
    // DoTrial(-5.12, 5.12, 0.01, DeJong1);

    // std::cout << " ========== DeJong2 ==========\n";
    // DoTrial(-2.048, 2.048, 0.001, DeJong2);

    // std::cout << " ========== DeJong3 ==========\n";
    // DoTrial(-5.12, 5.12, 0.01, DeJong3);

    // std::cout << " ========== DeJong4 ==========\n";
    // DoTrial(-1.28, 1.28, 0.01, DeJong4);

    // ... just don't run this one
    // std::cout << " ========== DeJong5 ==========\n";
    // DoTrial(-65.536, 65.536, 0.001, DeJong5);

    std::cout << "Max of DeJong1: " << FindRangeMax(-5.12, 5.12, 0.01, DeJong1) << "\n";
    std::cout << "Max of DeJong2: " << FindRangeMax(-2.048, 2.048, 0.001, DeJong2) << "\n";
    std::cout << "Max of DeJong3: " << FindRangeMax(-5.12, 5.12, 0.01, DeJong3) << "\n";
    std::cout << "Max of DeJong4: " << FindRangeMax(-1.28, 1.28, 0.01, DeJong4) << "\n";
    std::cout << "Max of DeJong5: " << FindRangeMax(-65.536, 65.536, 0.001, DeJong5) << "\n";

    return 0;
}

void PrintTrial(std::vector<double>& vec, double res)
{
    std::cout << "f(";
    std::cout << "<" << vec[0] << ", " << vec[1] << ">";
    std::cout << ") = ";
    std::cout << res << "\n";
}

void DoTrial(double min, double max, double step, double (*dejongFunc)(std::vector<double>&))
{
    std::vector<double> x(2, 0.0);
    x[0] = min;
    while (x[0] < max)
    {
        x[1] = min;
        while (x[1] < max)
        {
            double res = dejongFunc(x);
            PrintTrial(x, res);
            x[1] += step;
        }
        x[0] += step;
    }
}

double FindRangeMax(double min, double max, double step, double (*dejongFunc)(std::vector<double>&))
{
    std::vector<double> x(2, 0.0);
    double maxVal = std::numeric_limits<double>::lowest();
    x[0] = min;
    while (x[0] < max)
    {
        x[1] = min;
        while (x[1] < max)
        {
            double res = dejongFunc(x);
            if (res > maxVal) maxVal = res;
            x[1] += step;
        }
        x[0] += step;
    }

    return maxVal;
}