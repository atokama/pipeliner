#pragma once

#include <filesystem>
#include <cstdint>
#include <vector>
#include <chrono>

#include <csv.h>

#include <pipeliner/basic_block.h>

namespace pipeliner {

    using namespace std::chrono_literals;

    class GeneratorBlock : public BasicBlock {
    public:
        DataChunk waitChunk() {
            DataChunk c{};
            queue_.wait_dequeue(c);
            return std::move(c);
        }

    protected:
        moodycamel::BlockingReaderWriterQueue<DataChunk> queue_;
    };

    class CsvReaderBlock : public GeneratorBlock {
    public:
        CsvReaderBlock(const std::filesystem::path &csvFile,
                       std::chrono::duration<double> delay = 500ns);

        bool processChunk(bool shouldStop) override;

    private:
        const std::chrono::duration<double, std::nano> delay_;
        io::CSVReader<16> reader_;
        std::vector<Uint8> row_;
        std::size_t iter_;
    };

    class RandomNumberGeneratorBlock : public GeneratorBlock {
    public:
        RandomNumberGeneratorBlock(std::chrono::duration<double> delay = 500ns);

        bool processChunk(bool shouldStop) override;

        DataChunk process();

    private:
        const std::chrono::duration<double, std::nano> delay_;
    };

}