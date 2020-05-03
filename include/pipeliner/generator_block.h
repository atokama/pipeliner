#pragma once

#include <filesystem>
#include <cstdint>
#include <vector>

#include <csv.h>

#include <pipeliner/basic_block.h>

namespace pipeliner {

    class CsvReaderBlock : public BasicBlock {
    public:
        CsvReaderBlock(const std::filesystem::path &csvFile);

        std::unique_ptr<DataChunk> processChunk(std::unique_ptr<DataChunk>) override;

    private:
        io::CSVReader<16> reader_;
        std::vector<Uint8> row_;
        std::size_t iter_;
    };

    class RandomNumberGeneratorBlock : public BasicBlock {
    public:
        RandomNumberGeneratorBlock();

        std::unique_ptr<DataChunk> processChunk(std::unique_ptr<DataChunk>) override;
    };

}