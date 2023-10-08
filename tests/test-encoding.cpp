#include <iomanip>
#include <iostream>

#include "../encoding.hpp"

int main()
{
    std::cout << std::fixed << std::setprecision(3);

    float x = 0.0f;
    while (x < 102.3f)
    {
        std::cout << x << "\t=>\t";
        auto bitstring = EncodeFloat(x);
        for (auto val : bitstring)
            std::cout << static_cast<int>(val);
        std::cout << "\t=>\t";
        float decoded = DecodeFloat(bitstring);
        std::cout << decoded << "\n";
        x += 0.1f;
    }

    std::cout << "\n";

    Room living{0.1f, 0.2f, 0.3f, 0.4f};
    Room kitchen{0.5f, 0.6f, 0.7f, 0.8f};
    Room bath{0.9f, 1.0f, 1.1f, 1.2f};
    Room hall{1.3f, 1.4f, 1.5f, 1.6f};
    Room bed1{1.7f, 1.8f, 1.9f, 2.0f};
    Room bed2{2.1f, 2.2f, 2.3f, 2.4f};
    Room bed3{2.5f, 2.6f, 2.7f, 2.8f};
    std::array rooms{living, kitchen, bath, hall, bed1, bed2, bed3};

    auto chromosome = EncodeChromosome(rooms);
    for (int i = 0; i < NUM_ROOMS; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < FLOAT_BITWIDTH; k++)
            {
                std::cout << static_cast<int>(
                    chromosome[(i * ROOM_BITWIDTH) + (j * FLOAT_BITWIDTH) + k]);
            }
            std::cout << " | ";
        }
        std::cout << "\n";
    }

    auto decodedChromosome = DecodeChromosome(chromosome);
    for (Room& room : decodedChromosome)
        std::cout << room.length << " " << room.width << " " << room.x << " " << room.y << "\n";

    auto encoded5p5 = EncodeFloat(5.5f);
    float decoded5p5 = DecodeFloat(encoded5p5);

    std::cout << std::setprecision(17);
    std::cout << "5.5f literal value: " << 5.5f << "\n";
    std::cout << "5.5f decoded value: " << decoded5p5 << "\n";

    return 0;
}