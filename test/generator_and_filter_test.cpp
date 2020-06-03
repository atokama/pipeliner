#include <pipeliner/generator_block.h>
#include <pipeliner/filter_block.h>

#include <iostream>

#include "catch.hpp"

namespace pipeliner {

    TEST_CASE("RandomNumberGenerator", "[GeneratorAndFilterTest]") {
        RandomNumberGeneratorBlock b1{5000ns};
        FilterBlock b2{128, &b1};

        b1.debug().enable();
        b2.debug().enable();

        b2.start();

        for (int i = 0; i != 10; ++i) {
            b2.waitChunk();
        }

        b2.stop();

        std::string l1, l2;
        do {
            auto l1 = b1.debug().popLine();
            auto l2 = b2.debug().popLine();
            std::cout << l1 << " | " << l2 << std::endl;
        } while (l1.size() != 0 && l2.size() != 0);

        REQUIRE(b1.lostChunksCount() == 0);
        REQUIRE(b2.lostChunksCount() == 0);
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