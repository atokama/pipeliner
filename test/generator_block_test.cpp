#include <pipeliner/basic_block.h>

#include <filesystem>
#include <cstdint>
#include <vector>
#include <iostream>

#include <csv.h>

#include "catch.hpp"

namespace pipeliner {

    class CsvReaderBlock : public BasicBlock {
    public:
        CsvReaderBlock(const std::filesystem::path &csvFile)
                : BasicBlock{nullptr}, reader_{csvFile.string()}, row_{}, iter_{16} {
            row_.resize(16);
        }

        std::unique_ptr<DataChunk> processChunk(std::unique_ptr<DataChunk>) override {
            if (iter_ == row_.size()) {
                iter_ = 0;
                if (!reader_.read_row(
                        row_[0], row_[1], row_[2], row_[3],
                        row_[4], row_[5], row_[6], row_[7],
                        row_[8], row_[9], row_[10], row_[11],
                        row_[12], row_[13], row_[14], row_[15])) {
                    return std::make_unique<DataChunk>(DataChunk::End);
                }
            }

            auto chunk = std::make_unique<DataChunk>();
            chunk->data1 = row_[iter_++];
            chunk->data2 = row_[iter_++];
            return std::move(chunk);
        }

    private:
        io::CSVReader<16> reader_;
        std::vector<Uint8> row_;
        std::size_t iter_;
    };

    TEST_CASE("CsvReaderBlock", "[GeneratorBlock]") {
        SECTION("Reading 16 columns, 100 rows csv file") {
            const auto csvFile = std::filesystem::path{} / "data" / "MOCK_DATA_16_100.csv";

            SECTION("Check first 5 rows of csv file") {
                std::vector<Uint8> expected{
                        1, 114, 102, 69, 4, 169, 113, 58, 172, 115, 88, 177, 199, 212, 159, 22,
                        130, 233, 72, 31, 121, 34, 226, 49, 235, 214, 17, 120, 146, 115, 226, 143,
                        102, 189, 20, 86, 131, 201, 29, 67, 253, 65, 102, 143, 181, 101, 70, 228,
                        145, 35, 131, 221, 164, 134, 140, 249, 149, 75, 252, 59, 69, 7, 99, 179,
                        218, 242, 204, 3, 77, 32, 18, 42, 202, 161, 112, 226, 206, 51, 6, 43};

                CsvReaderBlock block{csvFile};

                std::vector<Uint8> actual{};
                while (expected.size() != actual.size()) {
                    auto chunk = block.processChunk(nullptr);
                    actual.push_back(chunk->data1);
                    actual.push_back(chunk->data2);
                }

                REQUIRE(expected == actual);
            }

            SECTION("Read file until the end") {
                CsvReaderBlock block{csvFile};
                int elementsCount{0};
                while (true) {
                    auto chunk = block.processChunk(nullptr);
                    if (chunk->getType() == DataChunk::End) {
                        break;
                    }
                    elementsCount += 2;
                }
                // 16 columns, 100 rows
                REQUIRE(16 * 100 == elementsCount);
            }
        }
    }

    class RandomNumberGeneratorBlock : public BasicBlock {
    public:
        RandomNumberGeneratorBlock() : BasicBlock{nullptr} {

        }
    };

}