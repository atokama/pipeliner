#include <pipeliner/generator_block.h>
#include <pipeliner/filter_block.h>

#include "catch.hpp"

namespace pipeliner {

    TEST_CASE("RandomNumberGenerator", "[GeneratorAndFilterTest]") {
        RandomNumberGeneratorBlock b1{};
        FilterBlock b2{128, &b1};

        b2.start();

        for (int i = 0; i != 100; ++i) {
            b2.waitChunk();
        }

        b2.stop();
    }

    TEST_CASE("CsvReader", "[GeneratorAndFilterTest]") {
        const auto csvFile = std::filesystem::path{} / "data" / "MOCK_DATA_16_100.csv";
        CsvReaderBlock b1{csvFile};
        FilterBlock b2{128, &b1};

        b2.start();

        while (true) {
            auto chunk = b2.waitChunk();
            if (chunk->getType() == DataChunk::Data) { break; }
        }

        b2.stop();
    }
}