#include <pipeliner/generator_block.h>
#include <pipeliner/filter_block.h>

#include <iostream>

#include "catch.hpp"

namespace pipeliner {

    const auto generatorBlockDelay = 0ms;
    const Size width = 16;
    const double threshold = 128;

    TEST_CASE("RandomNumberGenerator", "[GeneratorAndFilterTest]") {
        GeneratorBlock b1{generatorBlockDelay};
        FilterBlock b2{&b1, threshold, width};

        b2.start();

        for (int i = 0; i != 10; ++i) {
            FilteredChunk f{};
            b2.getProcessor().getQueue().wait_dequeue(f);
        }

        b2.stop();

        std::string l2;
        do {
            l2 = b2.getProcessor().debug().popLine();
            std::cout << l2 << std::endl;
        } while (l2.size() != 0);
    }

    TEST_CASE("CsvReader", "[GeneratorAndFilterTest]") {
        const auto csvFile = Path{} / "data" / "MOCK_DATA_16_100.csv";

        GeneratorBlock b1{csvFile, generatorBlockDelay};
        FilterBlock b2{&b1, threshold, width};

        b2.start();

        Size filtElementsCount{0};

        while (true) {
            FilteredChunk f{};
            b2.getProcessor().getQueue().wait_dequeue(f);
            if (f.getType() == DataChunk::End) {
                break;
            } else if (f.getType() == DataChunk::Data) {
                filtElementsCount += 2;
            }
        }

        b2.stop();

        REQUIRE(filtElementsCount == 800); // (16 - 4 - 4) * 100
    }
}