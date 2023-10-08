#include "encoding.hpp"

#include <cmath>
#include <cassert>

// encoding scheme is to chop the last positive value off to avoid increasing the bit width
//
// decoded = min + val * precision
// val = binary to decimal conversion
// precision = [max-min]/(2^n - 1)
// so....
// min = -5.12
// max-min = 5.11+5.12 = 10.23
// 2^10 - 1 = 1023
//
// precision = 10.23 / 1023 = 0.01
// 
// decoded = -5.12 + binary_to_decimal(bitstring) * 0.01
// decoded + 5.12 = binary_to_decimal(bitstring) * 0.01
// (decoded + 5.12) / 0.01 = binary_to_decimal(bitstring)
// decimal_to_binary((decoded + 5.12) / 0.01) = bitstring


std::vector<uint8_t> IntToBitstring(int x, int bitwidth)
{
    std::vector<uint8_t> ret(bitwidth, 0);
    for (int i = bitwidth - 1; i >= 0; i--)
    {
        ret[i] = x % 2;
        x /= 2;
    }
    return ret;
}

int BitstringToInt(std::vector<uint8_t> x)
{
    int ret = 0;
    for (int i = 0; i < x.size(); i++)
    {
        ret += static_cast<int>(x[i]) << (x.size() - i - 1);
    }
    return ret;
}

std::vector<uint8_t> Encode8(double x)
{
    // [-1.28, 1.28) => 2^8 values
    std::vector<uint8_t> ret(8, 0);
    x += 1.28;
    x *= 100.0;
    // casting by itself is a truncation, and sometime the doubles will be something like 0.9999999
    // hence the round being required
    int x_int = static_cast<int>(std::round(x));
    return IntToBitstring(x_int, 8);
}

std::vector<uint8_t> Encode10(double x)
{
    // [-5.12, 5.12) => 2^10 values
    std::vector<uint8_t> ret(10, 0);
    x += 5.12;
    x *= 100;
    int x_int = static_cast<int>(std::round(x));

    return IntToBitstring(x_int, 10);
}

std::vector<uint8_t> Encode12(double x)
{
    // [-2.048, 2.048) => 2^12 values
    std::vector<uint8_t> ret(12, 0);
    x += 2.048;
    x *= 1000.0;
    int x_int = static_cast<int>(std::round(x));
    return IntToBitstring(x_int, 12);
}

std::vector<uint8_t> Encode17(double x)
{    
    // [-65.536, 65.536) => 2^17 values
    std::vector<uint8_t> ret(17, 0);
    x += 65.536;
    x *= 1000.0;
    int x_int = static_cast<int>(std::round(x));
    return IntToBitstring(x_int, 17);
}

double Decode8(std::vector<uint8_t>& x)
{
    int x_int = BitstringToInt(x);
    x_int -= 128;
    double res = static_cast<double>(x_int) * 0.01;
    return res;
}

double Decode10(std::vector<uint8_t>& x)
{
    int x_int = BitstringToInt(x);
    x_int -= 512;
    double res = static_cast<double>(x_int) * 0.01;
    return res;
}

double Decode12(std::vector<uint8_t>& x)
{
    int x_int = BitstringToInt(x);
    x_int -= 2048;
    double res = static_cast<double>(x_int) * 0.001;
    return res;
}

double Decode17(std::vector<uint8_t>& x)
{
    int x_int = BitstringToInt(x);
    x_int -= 65536;
    double res = static_cast<double>(x_int) * 0.001;
    return res;
}


