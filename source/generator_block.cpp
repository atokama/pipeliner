#include <pipeliner/generator_block.h>

#include <cstdlib>
#include <ctime>

namespace pipeliner {

    CsvReaderBlock::CsvReaderBlock(const std::filesystem::path &csvFile,
                                   std::chrono::duration<double> delay)
            : delay_{delay},
              reader_{csvFile.string()}, row_{}, iter_{16} { row_.resize(16); }

    bool CsvReaderBlock::processChunk(bool shouldStop) {
        std::this_thread::sleep_for(delay_);

        if (iter_ == row_.size()) {
            iter_ = 0;
            if (!reader_.read_row(
                    row_[0], row_[1], row_[2], row_[3],
                    row_[4], row_[5], row_[6], row_[7],
                    row_[8], row_[9], row_[10], row_[11],
                    row_[12], row_[13], row_[14], row_[15])) {
                shouldStop = true;
            }
        }

        if (shouldStop) {
            queue_.enqueue(DataChunk{DataChunk::End});
            return false;
        }

        DataChunk chunk{};
        chunk.data1 = row_[iter_++];
        chunk.data2 = row_[iter_++];
        queue_.enqueue(std::move(chunk));
        return true;
    }

    RandomNumberGeneratorBlock::RandomNumberGeneratorBlock(std::chrono::duration<double> delay)
            : delay_{delay} { std::srand(std::time(nullptr)); }

    bool RandomNumberGeneratorBlock::processChunk(bool shouldStop) {
        std::this_thread::sleep_for(delay_);

        if (shouldStop) {
            queue_.enqueue(std::move(DataChunk{DataChunk::End}));
            return false;
        }

        auto chunk = process();

        queue_.enqueue(std::move(chunk));
        return true;

    }

    DataChunk RandomNumberGeneratorBlock::process() {
        DataChunk chunk{};
        chunk.data1 = std::rand() % std::numeric_limits<Uint8>::max();
        chunk.data2 = std::rand() % std::numeric_limits<Uint8>::max();
        return chunk;
    }

}
