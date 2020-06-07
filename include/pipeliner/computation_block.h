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


    class ComputationBlock : public BasicBlock {
    public:
        ComputationBlock(BasicBlock *prev) : BasicBlock{prev} {}

        std::unique_ptr<DataChunk>
        processChunk(std::unique_ptr<DataChunk> chunk) override {
            auto computedChunk = std::make_unique<ComputedChunk>();

            if (auto labelledChunk = dynamic_cast<LabelledChunk *>(chunk.get())) {
                auto pos1 = labelledChunk->pos;
                auto pos2 = labelledChunk->pos;
                pos2.col += 1;

                processLabel(labelledChunk->labels[0], pos1);
                processLabel(labelledChunk->labels[1], pos2);

                for (const auto &merge : labelledChunk->merges) { processMerge(merge, *labelledChunk->labelSet); }

                processFinishedLabels(pos1, *labelledChunk->labelSet, computedChunk->labelData);
                processFinishedLabels(pos2, *labelledChunk->labelSet, computedChunk->labelData);
            }

            // Finish all labels on the end
            if (chunk->getType() == DataChunk::End) {
                computedChunk->setType(DataChunk::End);
                for (auto iter = computedLabels_.begin(); iter != computedLabels_.end();) {
                    computedChunk->labelData.push_back(iter->second);
                    iter = computedLabels_.erase(iter);
                }
            }

            for (const auto &ld : computedChunk->labelData) {
                PILI_DEBUG_ADDTEXT(
                        "label:" << ld.label << " size:" << ld.size
                                 << " topLeft:" << ld.rect.topLeft.row << "," << ld.rect.topLeft.col
                                 << " bottomRight:" << ld.rect.bottomRight.row << "," << ld.rect.bottomRight.col);
                PILI_DEBUG_NEWLINE();
            }

            if (computedChunk->labelData.empty() && chunk->getType() != DataChunk::End) {
                return nullptr;
            }

            return std::move(computedChunk);
        }

        void processLabel(Uint16 label, const Pos &pos) {
            if (label == 0) {
                return;
            } else if (computedLabels_.count(label)) {
                auto &c = computedLabels_.at(label);
                ++c.size;
                if (pos.col < c.rect.topLeft.col) { c.rect.topLeft.col = pos.col; }
                if (pos.col > c.rect.bottomRight.col) { c.rect.bottomRight.col = pos.col; }
                if (pos.row < c.rect.topLeft.row) { c.rect.topLeft.row = pos.row; }
                if (pos.row > c.rect.bottomRight.row) { c.rect.bottomRight.row = pos.row; }
            } else {
                LabelData c{};
                c.label = label;
                c.size = 1;
                c.rect.topLeft = c.rect.bottomRight = pos;
                auto p = computedLabels_.insert({label, c});
                assert(p.second == true);
            }
        }

        void processMerge(const Merge &merge, LabelSet &labelSet) {
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
                c1.rect.bottomRight.col = c1.rect.bottomRight.col > c1.rect.bottomRight.col ?
                                          c1.rect.bottomRight.col : c2.rect.bottomRight.col;
                c1.rect.bottomRight.row = c1.rect.bottomRight.row > c2.rect.bottomRight.row ?
                                          c1.rect.bottomRight.row : c2.rect.bottomRight.row;

                // Recycle label after merge
                labelSet.put(c2.label);

                computedLabels_.erase(merge.label2);
            }
        }

        void processFinishedLabels(const Pos &pos, LabelSet &labelSet,
                                   std::vector<LabelData> &finishedLabels) {
            for (auto iter = computedLabels_.begin(); iter != computedLabels_.end();) {
                auto &c = iter->second;
                if (pos.row > c.rect.bottomRight.row + 1) {
                    // Recycle finished label
                    labelSet.put(c.label);

                    finishedLabels.push_back(c);
                    iter = computedLabels_.erase(iter);
                } else {
                    ++iter;
                }
            }
        }

    private:
        std::map<Uint16, LabelData> computedLabels_;
    };

}
