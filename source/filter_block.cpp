#include <pipeliner/filter_block.h>

namespace pipeliner {

    const std::list<double> FilterBlock::mul_ =
            {0.00025177, 0.008666992, 0.078025818,
             0.24130249, 0.343757629, 0.24130249,
             0.078025818, 0.008666992, 0.000125885};

    std::unique_ptr<DataChunk> FilterBlock::processChunk(std::unique_ptr<DataChunk> chunk) {
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
            filteredChunk->filt1 = value1 >= thresholdValue_;
            filteredChunk->filt2 = value2 >= thresholdValue_;
            return std::move(filteredChunk);
        }
        return nullptr;
    }

    double FilterBlock::computeValue(std::list<Uint8>::const_iterator bufIter) {
        double value{0};
        for (auto mulIter = mul_.cbegin(); mulIter != mul_.cend(); ++bufIter, ++mulIter) {
            value += (*mulIter) * (*bufIter);
        }
        return value;
    }

}