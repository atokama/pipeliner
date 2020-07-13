#pragma once

#include <queue>
#include <list>
#include <vector>

#include <pipeliner/basic_block.h>
#include "generator_block.h"

namespace pipeliner {

    using Size = std::size_t;

    struct Pos {
        Size row{};
        Size col{};
    };

    class FilteredChunk : public DataChunk {
    public:
        FilteredChunk(DataChunk::Type type = DataChunk::Data) : DataChunk{type} {}

        bool filt1{}, filt2{};
    };

    struct FilterContext {
        const Size width_;
        const double thresholdValue_;
        const std::list<double> mul_;
        Pos pos_;
        std::list<Uint8> buf_;
    };

    struct FilterProcessor : GenericProcessor<FilterProcessor, FilteredChunk, DataChunk>,
                             private FilterContext {
        FilterProcessor(double thresholdValue, Size width)
                : FilterContext{width, thresholdValue,
                                {0.00025177, 0.008666992, 0.078025818,
                                 0.24130249, 0.343757629, 0.24130249,
                                 0.078025818, 0.008666992, 0.000125885}, {0, 0}} {}

        FilteredChunk processChunkImpl(DataChunk c) {
            FilteredChunk chunk{DataChunk::Empty};

            buf_.push_back(c.data1);
            buf_.push_back(c.data2);
            if (buf_.size() == 10) {
                chunk.setType(DataChunk::Data);

                double value1 = computeValue(buf_.cbegin());
                double value2 = computeValue(++buf_.cbegin());
                buf_.pop_front();
                buf_.pop_front();

                chunk.data1 = c.data1;
                chunk.data2 = c.data2;
                chunk.filt1 = value1 >= thresholdValue_;
                chunk.filt2 = value2 >= thresholdValue_;

                PILI_DEBUG_ADDTEXT((chunk.filt1 ? 'x' : '_') << (chunk.filt2 ? 'x' : '_'));
            }

            pos_.col += 2;
            if (pos_.col >= width_) {
                PILI_DEBUG_ADDTEXT("; " << pos_.row);
                PILI_DEBUG_NEWLINE();
                pos_.col = 0;
                ++pos_.row;
                buf_.clear();
            }

            if (c.getType() == DataChunk::End) { chunk.setType(DataChunk::End); }
            return chunk;
        }

        double computeValue(std::list<Uint8>::const_iterator buf) {
            double value{0};
            for (auto iter = mul_.cbegin(); iter != mul_.cend(); ++buf, ++iter) {
                value += (*iter) * (*buf);
            }
            return value;
        }
    };

    using FilterBlock = GenericBlock<FilterProcessor>;

}
