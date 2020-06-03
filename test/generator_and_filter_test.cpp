#include <pipeliner/generator_block.h>
#include <pipeliner/filter_block.h>

#include <iostream>

#include "catch.hpp"

namespace pipeliner {

    TEST_CASE("RandomNumberGenerator", "[GeneratorAndFilterTest]") {
        RandomNumberGeneratorBlock b1{};
        FilterBlock b2{128, &b1};

        b1.debug().enable();
        b2.debug().enable();

        b2.start();

        for (int i = 0; i != 40; ++i) {
            b2.waitChunk();
        }

        b2.stop();

        for (auto l1 = b1.debug().popLine(), l2 = b2.debug().popLine();
                l1.size() != 0 && l2.size() != 0; ) {
           std::cout << l1 << " | " << l2 << std::endl;
        }
    }

    TEST_CASE("CsvReader", "[GeneratorAndFilterTest]") {
        const auto csvFile = std::filesystem::path{} / "data" / "MOCK_DATA_16_100.csv";
        CsvReaderBlock b1{csvFile};
        FilterBlock b2{128, &b1};

        b2.start();

        while (true) {
            auto chunk = b2.waitChunk();
            if (chunk->getType() == DataChunk::End) { break; }
        }

        b2.stop();
    }
}