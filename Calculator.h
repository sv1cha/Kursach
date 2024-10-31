#pragma once
#include <vector>
#include <iostream>
#include <limits>
#include <boost/numeric/conversion/cast.hpp>

class Calculator
{
    int64_t results;
public:
    Calculator(std::vector<int64_t> input_data);
    int64_t send_res();
};
