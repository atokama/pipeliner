#pragma once

#include <filesystem>
#include <cstdint>
#include <vector>
#include <chrono>

#include <csv.h>

#include <pipeliner/basic_block.h>

namespace pipeliner {

    using namespace std::chrono_literals;
    using Path = std::filesystem::path;

    struct GeneratorContext {
        struct CsvReader {
            CsvReader(const Path &csvFile) : reader_{csvFile.string()}, row_{}, iter_{16} {}

            io::CSVReader<16> reader_;
            std::vector<Uint8> row_;
            std::size_t iter_;
        };

        const std::chrono::duration<double, std::nano> delay_;
        std::unique_ptr<CsvReader> csvReader_{nullptr};
    };

    struct GeneratorProcessor : GenericProcessor<GeneratorProcessor, DataChunk, DataChunk>,
                                private GeneratorContext {
        GeneratorProcessor(const Path &csvFile, std::chrono::duration<double> delay = 500ns)
                : GeneratorContext{delay, std::make_unique<CsvReader>(csvFile)} {
            csvReader_->row_.resize(16);
        }

        GeneratorProcessor(std::chrono::duration<double> delay = 500ns)
                : GeneratorContext{delay} {
            std::srand(std::time(nullptr));
        }

        DataChunk processChunkImpl(DataChunk c) {
            std::this_thread::sleep_for(delay_);

            DataChunk chunk{};

            if (csvReader_) {
                auto &iter{csvReader_->iter_};
                auto &row{csvReader_->row_};

                if (iter == row.size()) {
                    iter = 0;
                    if (!csvReader_->reader_.read_row(
                            row[0], row[1], row[2], row[3],
                            row[4], row[5], row[6], row[7],
                            row[8], row[9], row[10], row[11],
                            row[12], row[13], row[14], row[15])) {
                        return DataChunk{DataChunk::End};
                    }
                }

                chunk.data1 = row[iter++];
                chunk.data2 = row[iter++];

            } else {
                const auto max = std::numeric_limits<Uint8>::max();
                chunk.data1 = std::rand() % max;
                chunk.data2 = std::rand() % max;
            }

            if (c.getType() == DataChunk::End) { chunk.setType(DataChunk::End); }
            return chunk;
        }

    };

    using GeneratorBlock = GenericBlock<GeneratorProcessor>;

}