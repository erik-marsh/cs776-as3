#include <iomanip>
#include <iostream>

#include "../encoding.hpp"

int main()
{
    double vals[] = {-1.28, -5.12, -2.048, -65.536};
    double deltas[] = {0.01, 0.01, 0.001, 0.001};
    std::vector<uint8_t> (*encodes[])(double) = {Encode8, Encode10, Encode12, Encode17};
    double (*decodes[])(std::vector<uint8_t>&) = {Decode8, Decode10, Decode12, Decode17};

    std::cout << std::fixed << std::setprecision(4);
    for (int i = 0; i < 4; i++)
    {
        std::cout << "\n\n ========== TRIAL " << i << " ==========\n";
        double x = vals[i];
        while (x < -vals[i])
        {
            std::cout << x << "\t=>\t";
            auto bs = encodes[i](x);
            for (auto val : bs)
                std::cout << (int)val;
            std::cout << "\t=>\t";
            double d = decodes[i](bs);
            std::cout << d << "\n";
            x += deltas[i];
        }
    }

    return 0;
}