#include <pipeliner/filter_block.h>
#include <pipeliner/debug.h>
#include <pipeliner/generator_block.h>

namespace pipeliner {

    const std::list<double> FilterBlock::mul_ =
            {0.00025177, 0.008666992, 0.078025818,
             0.24130249, 0.343757629, 0.24130249,
             0.078025818, 0.008666992, 0.000125885};

    FilteredChunk FilterBlock::process(const DataChunk &chunk) {
        FilteredChunk filteredChunk{DataChunk::Empty};

        buf_.push_back(chunk.data1);
        buf_.push_back(chunk.data2);
        if (buf_.size() == 10) {
            filteredChunk.setType(DataChunk::Data);

            double value1 = computeValue(buf_.cbegin());
            double value2 = computeValue(++buf_.cbegin());
            buf_.pop_front();
            buf_.pop_front();

            filteredChunk.data1 = chunk.data1;
            filteredChunk.data2 = chunk.data2;
            filteredChunk.filt1 = value1 >= thresholdValue_;
            filteredChunk.filt2 = value2 >= thresholdValue_;

            PILI_DEBUG_ADDTEXT((filteredChunk.filt1 ? 'x' : '_') << (filteredChunk.filt2 ? 'x' : '_'));
        }

        pos_.col += 2;
        if (pos_.col >= width_) {
            PILI_DEBUG_ADDTEXT("; " << pos_.row);
            PILI_DEBUG_NEWLINE();
            pos_.col = 0;
            ++pos_.row;
            buf_.clear();
        }

        return filteredChunk;
    }

    bool FilterBlock::processChunk(bool shouldStop) {
        auto chunk = dynamic_cast<GeneratorBlock *>(prevBlock_)->waitChunk();

        if (shouldStop || chunk.getType() == DataChunk::End) {
            queue_.enqueue(FilteredChunk{DataChunk::End});
            return false;
        }

        auto filteredChunk = process(std::move(chunk));

        queue_.enqueue(std::move(filteredChunk));
        return true;
    }

    double FilterBlock::computeValue(std::list<Uint8>::const_iterator bufIter) {
        double value{0};
        for (auto mulIter = mul_.cbegin(); mulIter != mul_.cend(); ++bufIter, ++mulIter) {
            value += (*mulIter) * (*bufIter);
        }
        return value;
    }

}