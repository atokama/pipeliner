#include <pipeliner/basic_block.h>

#include <filesystem>
#include <cstdint>
#include <vector>
#include <iostream>

#include <csv.h>

#include "catch.hpp"

namespace pipeliner {

    using Path = std::filesystem::path;
    using Uint8 = std::uint8_t;

    TEST_CASE("GeneratorBlock", "[GeneratorBlock]") {
        io::CSVReader<16> in{
                (Path{} / "data" / "MOCK_DATA_16_100.csv").string()};
        //in.read_header(io::ignore_extra_column, "vendor", "size", "speed");
        std::vector<Uint8> row;
        row.resize(16);
        while (in.read_row(
                row[0], row[1], row[2], row[3],
                row[4], row[5], row[6], row[7],
                row[8], row[9], row[10], row[11],
                row[12], row[13], row[14], row[15])) {
            std::cout << "row:";
            for (const auto item : row) {
                std::cout << " " << int{item};
            }
            std::cout << std::endl;
        }
    }

}