#include <pipeliner/computation_block.h>
#include <pipeliner/generator_block.h>

#include <iostream>

#include "catch.hpp"

namespace pipeliner {

    TEST_CASE("ComputationBlock", "[ComputationBlock]") {

        const auto generatorBlockDelay = 0ms;
        const int threshold = 128;
        const int width = 24;

        RandomNumberGeneratorBlock b1{generatorBlockDelay};
        FilterBlock b2{threshold, width, &b1};
        LabellingBlock b3{width - 8, &b2};
        ComputationBlock b4{&b3};

        b4.start();

        for (int i = 0; i != 10; ++i) {
            const auto computedChunk = dynamic_cast<ComputedChunk *>(b4.waitChunk().get());
            for (const auto &ld : computedChunk->labelData) {
                std::cout << "label:" << ld.label << " size:" << ld.size
                          << " topLeft:" << ld.rect.topLeft.row << "," << ld.rect.topLeft.col
                          << " bottomRight:" << ld.rect.bottomRight.row << "," << ld.rect.bottomRight.col
                          << "\n";
            }
        }

        b4.stop();

        std::string l2, l3;
        do {
            l2 = b2.debug().popLine();
            l3 = b3.debug().popLine();
            std::cout << l2 << " | " << l3 << std::endl;
        } while (l2.size() != 0 || l3.size() != 0);

    }
}