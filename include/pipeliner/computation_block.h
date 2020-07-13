#pragma once

#include <pipeliner/labelling_block.h>

#include <map>

namespace pipeliner {

    struct Rect {
        Pos topLeft;
        Pos bottomRight;
    };

    struct LabelData {
        Uint16 label = {};
        Size size = {};
        Rect rect = {};
    };

    class ComputedChunk : public DataChunk {
    public:
        ComputedChunk(DataChunk::Type type = DataChunk::Data) : DataChunk{type} {}

        std::vector<LabelData> labelData;
    };

    struct ComputationContext {
        std::map<Uint16, LabelData> computedLabels_;
        std::vector<Uint16> releasedLabels_;
    };

    struct ComputationProcessor : GenericProcessor<ComputationProcessor, ComputedChunk, LabelledChunk>,
                                  private ComputationContext {
        ComputedChunk processChunkImpl(LabelledChunk c) {
            ComputedChunk chunk{};

            auto pos1 = c.pos;
            auto pos2 = c.pos;
            pos2.col += 1;

            processFinishedLabels(pos1, chunk.labelData);
            processFinishedLabels(pos2, chunk.labelData);

            processLabel(c.labels[0], pos1);
            processLabel(c.labels[1], pos2);

            for (const auto &merge : c.merges) {
                processMerge(merge);
            }

            // Finish all labels on the end
            if (c.getType() == DataChunk::End) {
                for (auto iter = computedLabels_.begin(); iter != computedLabels_.end();) {
                    chunk.labelData.push_back(iter->second);
                    iter = computedLabels_.erase(iter);
                }
            }

            for (const auto &ld : chunk.labelData) {
                PILI_DEBUG_ADDTEXT(
                        "label:" << ld.label << " size:" << ld.size
                                 << " topLeft:" << ld.rect.topLeft.row << "," << ld.rect.topLeft.col
                                 << " bottomRight:" << ld.rect.bottomRight.row << "," << ld.rect.bottomRight.col);
                PILI_DEBUG_NEWLINE();
            }

            if (!releasedLabels_.empty()) {
                LabelledChunk l{};
                std::swap(l.releasedLabels, releasedLabels_);
                enqueueReverseChunk(std::move(l));
            }

            if (c.getType() == DataChunk::End) { chunk.setType(DataChunk::End); }
            return chunk;
        }

        void processLabel(Uint16 label, const Pos &pos) {
            if (label == 0) {
                return;
            } else if (computedLabels_.count(label)) {
                // Recompute existing label
                auto &c = computedLabels_.at(label);
                ++c.size;
                if (pos.col < c.rect.topLeft.col) { c.rect.topLeft.col = pos.col; }
                if (pos.col > c.rect.bottomRight.col) { c.rect.bottomRight.col = pos.col; }
                if (pos.row < c.rect.topLeft.row) { c.rect.topLeft.row = pos.row; }
                if (pos.row > c.rect.bottomRight.row) { c.rect.bottomRight.row = pos.row; }
            } else {
                // Add new label
                LabelData c{};
                c.label = label;
                c.size = 1;
                c.rect.topLeft = c.rect.bottomRight = pos;
                auto p = computedLabels_.insert({label, c});
                assert(p.second == true);
            }
        }

        void processMerge(const Merge &merge) {
            // Labelling block issues duplicate merges, so the merge could be already processed
            if (computedLabels_.count(merge.label1) &&
                computedLabels_.count(merge.label2)) {

                auto &c1 = computedLabels_.at(merge.label1);
                auto &c2 = computedLabels_.at(merge.label2);

                c1.size += c2.size;

                c1.rect.topLeft.col = c1.rect.topLeft.col < c2.rect.topLeft.col ?
                                      c1.rect.topLeft.col : c2.rect.topLeft.col;
                c1.rect.topLeft.row = c1.rect.topLeft.row < c2.rect.topLeft.row ?
                                      c1.rect.topLeft.row : c2.rect.topLeft.row;
                c1.rect.bottomRight.col = c1.rect.bottomRight.col > c2.rect.bottomRight.col ?
                                          c1.rect.bottomRight.col : c2.rect.bottomRight.col;
                c1.rect.bottomRight.row = c1.rect.bottomRight.row > c2.rect.bottomRight.row ?
                                          c1.rect.bottomRight.row : c2.rect.bottomRight.row;

                // Recycle label after merge
                releasedLabels_.push_back(c2.label);

                computedLabels_.erase(merge.label2);
            }
        }

        void processFinishedLabels(const Pos &pos, std::vector<LabelData> &finishedLabels) {
            for (auto iter = computedLabels_.begin(); iter != computedLabels_.end();) {
                auto &c = iter->second;
                if (pos.row > c.rect.bottomRight.row + 1) {
                    // Recycle finished label
                    releasedLabels_.push_back(c.label);

                    finishedLabels.push_back(c);
                    iter = computedLabels_.erase(iter);
                } else {
                    ++iter;
                }
            }
        }
    };

    using ComputationBlock = GenericBlock<ComputationProcessor>;

}
