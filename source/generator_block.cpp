#include <pipeliner/generator_block.h>

#include <cstdlib>
#include <ctime>

namespace pipeliner {

    CsvReaderBlock::CsvReaderBlock(const std::filesystem::path &csvFile)
            : BasicBlock{nullptr}, reader_{csvFile.string()}, row_{}, iter_{16} { row_.resize(16); }

    std::unique_ptr<DataChunk> CsvReaderBlock::processChunk(std::unique_ptr<DataChunk>) {
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

    RandomNumberGeneratorBlock::RandomNumberGeneratorBlock(std::chrono::duration<double> delayNs)
            : BasicBlock{nullptr}, delay_{delayNs} { std::srand(std::time(nullptr)); }

    std::unique_ptr<DataChunk> RandomNumberGeneratorBlock::processChunk(std::unique_ptr<DataChunk>) {
        std::this_thread::sleep_for(delay_);

        auto chunk = std::make_unique<DataChunk>();
        chunk->data1 = std::rand() % std::numeric_limits<Uint8>::max();
        chunk->data2 = std::rand() % std::numeric_limits<Uint8>::max();

        debug().addText(std::to_string(chunk->data1) + " " + std::to_string(chunk->data2) + " ");
        return std::move(chunk);
    }

}
