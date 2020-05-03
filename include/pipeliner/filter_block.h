#pragma once

#include <queue>
#include <list>
#include <vector>

#include <pipeliner/basic_block.h>

namespace pipeliner {

    class FilteredChunk : public DataChunk {
    public:
        FilteredChunk(DataChunk::Type type = DataChunk::Data) : DataChunk{type} {}

        bool filteredData1, filteredData2;
    };

    class FilterBlock : public BasicBlock {
    public:
        FilterBlock(double thresholdValue, BasicBlock *prev)
                : BasicBlock{prev}, thresholdValue_{thresholdValue} {
        }

        std::unique_ptr<DataChunk> processChunk(std::unique_ptr<DataChunk> chunk) override {
            if (chunk->getType() == DataChunk::End) {
                return nullptr;
            }

            buf_.push_back(chunk->data1);
            buf_.push_back(chunk->data2);
            if (buf_.size() == 10) {
                double value1 = computeValue(buf_.cbegin());
                double value2 = computeValue(++buf_.cbegin());
                buf_.pop_front();
                buf_.pop_front();

                auto filteredChunk = std::make_unique<FilteredChunk>();
                filteredChunk->data1 = chunk->data1;
                filteredChunk->data2 = chunk->data2;
                filteredChunk->filteredData1 = value1 >= thresholdValue_;
                filteredChunk->filteredData2 = value2 >= thresholdValue_;
                return std::move(filteredChunk);
            }
            return nullptr;
        }

    private:
        double computeValue(std::list<Uint8>::const_iterator bufIter) {
            double value{0};
            for (auto mulIter = mul_.cbegin(); mulIter != mul_.cend(); ++bufIter, ++mulIter) {
                value += (*mulIter) * (*bufIter);
            }
            return value;
        }

        std::list<Uint8> buf_;
        const double thresholdValue_;
        static const std::list<double> mul_;
    };
}
