#include "catch.hpp"
#include <iostream>
#include <pipeliner/labelling_block.h>
#include <pipeliner/generator_block.h>

namespace pipeliner {

    TEST_CASE("LabellingBlock", "[LabellingBlock]") {

        const auto generatorBlockDelay = 6ms;
        const int threshold = 160;
        const int width = 16;

        RandomNumberGeneratorBlock b1{generatorBlockDelay};
        FilterBlock b2{threshold, width, &b1};
        LabellingBlock b3{width - 8, &b2};

        b3.start();

        for (int i = 0; i != 32; ++i) {
            b3.waitChunk();
        }

        b3.stop();

        std::string l2, l3;
        do {
            l2 = b2.debug().popLine();
            l3 = b3.debug().popLine();
            std::cout << l2 << " | " << l3 << std::endl;
        } while (l2.size() != 0 || l3.size() != 0);

        REQUIRE(b1.lostChunksCount() == 0);
        REQUIRE(b2.lostChunksCount() == 0);
        REQUIRE(b3.lostChunksCount() == 0);
    }

}
