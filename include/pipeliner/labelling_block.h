#pragma once

#include <cstdint>

#include <pipeliner/filter_block.h>

namespace pipeliner {

    using Uint16 = std::uint16_t;

    struct Pos {
        Size row;
        Size col;
    };

    struct Merge {
        Uint16 label1;
        Uint16 label2;
    };

    class LabelledChunk : public DataChunk {
    public:
        LabelledChunk(DataChunk::Type type = DataChunk::Data) : DataChunk{type} {}

        Pos pos = {};
        Uint16 labels[2] = {0, 0};
        std::vector<Merge> merge;
    };

    class LabellingBlock : public BasicBlock {
    public:
        LabellingBlock(Size width, BasicBlock *prev)
                : BasicBlock{prev}, width_{width}, label_{1}, pos_{} {
            prevRow_.resize(width_);
            curRow_.resize(width_);
        }

        std::unique_ptr<DataChunk>
        processChunk(std::unique_ptr<DataChunk> chunk) override {
            if (chunk->getType() == DataChunk::End) {
                return nullptr;
            }
            auto filteredChunk = dynamic_cast<FilteredChunk *>(chunk.get());
            if (!filteredChunk) {
                return nullptr;
            }

            auto labelledChunk = std::make_unique<LabelledChunk>();

            processElement(filteredChunk->filt1);
            processElement(filteredChunk->filt2);

            labelledChunk->pos = pos_;
            labelledChunk->labels[0] = 0;
            labelledChunk->labels[1] = 1;

            PILI_DEBUG_ADDTEXT(0 << 1);

            pos_.col += 2;
            if (pos_.col >= width_) {
                pos_.col = 0;
                pos_.row += 1;
                PILI_DEBUG_NEWLINE();
            }

            return std::move(labelledChunk);
        }

        Uint16 processElement(bool elem) {
            if (!elem) { return 0; }

        }

    private:
        const Size width_;
        std::vector<Uint16> prevRow_, curRow_;
        Uint16 label_;
        Pos pos_;
    };
}
