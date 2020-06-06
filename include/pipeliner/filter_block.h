#pragma once

#include <queue>
#include <list>
#include <vector>

#include <pipeliner/basic_block.h>

namespace pipeliner {

    using Size = std::size_t;

    struct Pos {
        Size row;
        Size col;
    };

    class FilteredChunk : public DataChunk {
    public:
        FilteredChunk(DataChunk::Type type = DataChunk::Data) : DataChunk{type} {}

        bool filt1, filt2;
    };

    class FilterBlock : public BasicBlock {
    public:
        FilterBlock(double thresholdValue, Size width, BasicBlock *prev)
                : BasicBlock{prev}, thresholdValue_{thresholdValue},
                  pos_{0, 0}, width_{width} {}

        std::unique_ptr<DataChunk> processChunk(std::unique_ptr<DataChunk> chunk) override;

    private:
        double computeValue(std::list<Uint8>::const_iterator bufIter);

        std::list<Uint8> buf_;
        const double thresholdValue_;
        Pos pos_;
        const Size width_;
        static const std::list<double> mul_;
    };
}
