#include "Calculator.h"

Calculator::Calculator(std::vector<int64_t> input_data)
{
    try {
        results = 0;
        for(auto elem : input_data) {
            results += elem * elem;
            // Обработка переполнения
            if (results > INT64_MAX / 2) {
                results = INT64_MAX;
            } else if (results < INT64_MIN / 2) {
                results = INT64_MIN;
            }
        }
    } catch (boost::numeric::negative_overflow& e) {
        results = std::numeric_limits<int64_t>::lowest();
    } catch (boost::numeric::positive_overflow& e) {
        results = std::numeric_limits<int64_t>::max();
    }
}

int64_t Calculator::send_res() {
    return results;
}
