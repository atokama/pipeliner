#include <pipeliner/computation_block.h>
#include <pipeliner/generator_block.h>

#include <iostream>

#include "catch.hpp"

namespace pipeliner {

    TEST_CASE("ComputationBlock", "[ComputationBlock]") {

        const auto generatorBlockDelay = 0ms;
        const int threshold = 128;
        const int width = 32;

        RandomNumberGeneratorBlock b1{generatorBlockDelay};
        FilterBlock b2{threshold, width, &b1};
        LabellingBlock b3{width - 8, &b2};
        ComputationBlock b4{&b3};

        b4.start();

        for (int i = 0; i != 10; ++i) {
            b4.waitChunk();
        }

        b4.stop();

        std::string l2;
        do {
            l2 = b2.debug().popLine();
            std::cout << l2 << std::endl;
        } while (l2.size() != 0);

        std::cout << std::endl;

        std::string l4;
        do {
            l4 = b4.debug().popLine();
            std::cout << l4 << std::endl;
        } while (l4.size() != 0);
    }
}