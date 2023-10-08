#pragma once

#include <vector>
#include <cstdint>

std::vector<uint8_t> IntToBitstring(int x, int bitwidth);
int BitstringToInt(std::vector<uint8_t> x);

std::vector<uint8_t> Encode8(double x);
std::vector<uint8_t> Encode10(double x);
std::vector<uint8_t> Encode12(double x);
std::vector<uint8_t> Encode17(double x);

double Decode8(std::vector<uint8_t>& x);
double Decode10(std::vector<uint8_t>& x);
double Decode12(std::vector<uint8_t>& x);
double Decode17(std::vector<uint8_t>& x);