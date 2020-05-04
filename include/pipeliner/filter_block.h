#pragma once

#include <queue>
#include <list>
#include <vector>

#include <pipeliner/basic_block.h>

namespace pipeliner {

    class FilteredChunk : public DataChunk {
    public:
        FilteredChunk(DataChunk::Type type = DataChunk::Data) : DataChunk{type} {}

        bool filt1, filt2;
    };

    class FilterBlock : public BasicBlock {
    public:
        FilterBlock(double thresholdValue, BasicBlock *prev)
                : BasicBlock{prev}, thresholdValue_{thresholdValue} {
        }

        std::unique_ptr<DataChunk> processChunk(std::unique_ptr<DataChunk> chunk) override;

    private:
        double computeValue(std::list<Uint8>::const_iterator bufIter);

        std::list<Uint8> buf_;
        const double thresholdValue_;
        static const std::list<double> mul_;
    };
}
