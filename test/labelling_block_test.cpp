#include "catch.hpp"
#include <iostream>
#include <pipeliner/labelling_block.h>
#include <pipeliner/generator_block.h>

namespace pipeliner {

    TEST_CASE("LabellingBlock", "[LabellingBlock]") {

        const auto generatorBlockDelay = 0ms;
        const double threshold = 128;
        const Size width = 16;

        GeneratorBlock b1{generatorBlockDelay};
        FilterBlock b2{&b1, threshold, width};
        LabellingBlock b3{&b2, static_cast<Uint16>(width - 8)};

        b3.start();

        for (int i = 0; i != 32; ++i) {
            LabelledChunk c{};
            b3.getProcessor().getQueue().wait_dequeue(c);
        }

        b3.stop();

        std::string l2, l3;
        do {
            l2 = b2.getProcessor().debug().popLine();
            l3 = b3.getProcessor().debug().popLine();
            std::cout << l2 << " | " << l3 << std::endl;
        } while (l2.size() != 0 || l3.size() != 0);

    }

}
