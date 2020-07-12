#include <pipeliner/filter_block.h>

#include <iostream>

#include "catch.hpp"

namespace pipeliner {

    TEST_CASE("FilterBlock", "[FilterBlock]") {
        std::list<Uint8> input{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        const double thresholdValue = 8;
        const Size width = 16;

        FilterBlock filterBlock{thresholdValue, width, nullptr};

        std::vector<bool> output{};
        while (!input.empty()) {
            DataChunk chunk{};
            chunk.data1 = input.front();
            input.pop_front();
            chunk.data2 = input.front();
            input.pop_front();
            auto fc = filterBlock.process(std::move(chunk));
            output.push_back(fc.filt1);
            output.push_back(fc.filt2);
        }

        std::vector<bool> expected{false, false, false, false, true, true, true, true};
//        REQUIRE(expected == output);
    }

}