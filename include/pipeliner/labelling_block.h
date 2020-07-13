#pragma once

#include <cstdint>
#include <algorithm>
#include <set>
#include <mutex>
#include <cassert>

#include <pipeliner/filter_block.h>

namespace pipeliner {

    using Uint16 = std::uint16_t;

    struct Merge {
        Uint16 label1{};
        Uint16 label2{};
    };

    // LabelSet contains set of available labels.
    class LabelSet {
    public:
        LabelSet(Uint16 size) : next_{size} {
            ++next_;
            for (Uint16 label{1}; label != size + 1; ++label) {
                labels_.insert(label);
            }
        }

        Uint16 get() {
            if (labels_.empty()) {
                labels_.insert(next_++);
            }
            auto iter = labels_.begin();
            auto label = *iter;
            labels_.erase(iter);
            return label;
        }

        void put(Uint16 label) {
            auto p = labels_.insert(label);
        }

    private:
        std::set<Uint16> labels_;
        Uint16 next_;
    };

    class LabelledChunk : public DataChunk {
    public:
        LabelledChunk(DataChunk::Type type = DataChunk::Data) : DataChunk{type} {}

        Pos pos = {};
        Uint16 labels[2] = {0, 0};
        std::vector<Merge> merges;

        std::vector<Uint16> releasedLabels;
    };

    class LabelReuseChunk : public DataChunk {
    public:
        LabelReuseChunk() : DataChunk{DataChunk::Data} {}

    };

    struct LabellingContext {
        const Uint16 width_;
        LabelSet labelSet_;
        Pos pos_;
        std::vector<Uint16> prevRow_, curRow_;
    };

    struct LabellingProcessor : GenericProcessor<LabellingProcessor, LabelledChunk, FilteredChunk>,
                                private LabellingContext {
        LabellingProcessor(Uint16 width)
                : LabellingContext{width, LabelSet{width}, {}} {
            prevRow_.resize(width_, 0);
            curRow_.resize(width_, 0);
        }

        void processReverseChunkImpl(LabelledChunk c) {
            for (const auto label : c.releasedLabels) {
                labelSet_.put(label);
            }
        }

        LabelledChunk processChunkImpl(FilteredChunk c) {
            LabelledChunk chunk{};
            chunk.pos = pos_;

            chunk.labels[0] = processElement(c.filt1, chunk.merges);
            chunk.labels[1] = processElement(c.filt2, chunk.merges);

            for (const auto &merge : chunk.merges) {
                PILI_DEBUG_ADDTEXT("M(" << merge.label1 << "," << merge.label2 << ") ");
            }

            if (pos_.col >= width_) {
                pos_.col = 0;
                pos_.row += 1;
                std::swap(prevRow_, curRow_);
                PILI_DEBUG_NEWLINE();
            }

            PILI_DEBUG_ADDTEXT("; ");

            if (c.getType() == DataChunk::End) { chunk.setType(DataChunk::End); }
            return chunk;
        }

        Uint16 processElement(bool elem, std::vector<Merge> &merge) {
            Uint16 label{};
            Uint16 neibhors[] = {0, 0, 0, 0};

            if (elem) {
                // Here we consider west, north-west, north and north-east neibhors
                if (pos_.col != 0) {
                    neibhors[0] = curRow_[pos_.col - 1];
                    neibhors[1] = prevRow_[pos_.col - 1];
                }
                neibhors[2] = prevRow_[pos_.col];

                if (pos_.col + 1 != width_) {
                    neibhors[3] = prevRow_[pos_.col + 1];
                }

                const Uint16 minimum = findMinimumNonZero(neibhors);
                if (minimum == 0) {
                    // Element has no neibhors, assign new label
                    label = labelSet_.get();
                } else {
                    // Assign minimal label and merge with neibhors (there might be duplicate merges,
                    // which will be ignored by the ComputationBlock
                    label = minimum;
                    for (auto i{0}; i != 4; ++i) {
                        const auto n = neibhors[i];
                        if (n && label != n) {
                            merge.push_back(Merge{label, n});
                        }
                    }

                    updateNeibhorsLabels(label);
                }
            }

            if (label) {
                PILI_DEBUG_ADDTEXT(pos_.col << "(" << label << ") ");
            }

            curRow_[pos_.col] = label;
            ++pos_.col;
            return label;
        }

        void updateNeibhorsLabels(Uint16 label) {
            if (pos_.col != 0) {
                auto &n1 = curRow_[pos_.col - 1];
                if (n1 != 0) { n1 = label; }

                auto &n2 = prevRow_[pos_.col - 1];
                if (n2 != 0) { n2 = label; }
            }
            auto &n3 = prevRow_[pos_.col];
            if (n3 != 0) { n3 = label; }

            if (pos_.col + 1 != width_) {
                auto &n4 = prevRow_[pos_.col + 1];
                if (n4 != 0) { n4 = label; }
            }
        }

        Uint16 findMinimumNonZero(Uint16 *labels) {
            const auto max{std::numeric_limits<Uint16>::max()};
            auto min{max};
            for (auto i{0}; i != 4; ++i) {
                const auto l = labels[i];
                if (l == 0) { continue; }
                else if (l < min) { min = l; }
            }
            return min == max ? 0 : min;
        }
    };

    using LabellingBlock = GenericBlock<LabellingProcessor>;

}
