#include <pipeliner/computation_block.h>
#include <pipeliner/generator_block.h>

#include <iostream>

#include "catch.hpp"

namespace pipeliner {

    TEST_CASE("ComputationBlock", "[ComputationBlock]") {

        const auto generatorBlockDelay = 0ms;
        const double threshold = 128;
        const Size width = 32;

        GeneratorBlock b1{generatorBlockDelay};
        FilterBlock b2{&b1, threshold, width};
        LabellingBlock b3{&b2, static_cast<Uint16>(width - 8)};
        ComputationBlock b4{&b3};

        b4.start();

        for (int i = 0; i != 10; ++i) {
            ComputedChunk c{};
            b4.getProcessor().getQueue().wait_dequeue(c);
        }

        b4.stop();

        std::string l2;
        do {
            l2 = b2.getProcessor().debug().popLine();
            std::cout << l2 << std::endl;
        } while (l2.size() != 0);

        std::cout << std::endl;

        std::string l4;
        do {
            l4 = b4.getProcessor().debug().popLine();
            std::cout << l4 << std::endl;
        } while (l4.size() != 0);
    }

    struct FilterMockContext {
        std::queue<std::string> data_;
    };

    struct FilterProcessorMock : GenericProcessor<FilterProcessorMock, FilteredChunk, DataChunk>,
                                 private FilterMockContext {
        FilterProcessorMock(const std::queue<std::string> &data)
                : FilterMockContext{data} {}

        FilteredChunk processChunkImpl(DataChunk c) {
            FilteredChunk chunk{};

            if (!data_.empty()) {
                chunk.filt1 = setFilt(data_.front());
                chunk.filt2 = setFilt(data_.front());

                if (data_.front().empty()) { data_.pop(); }

            } else {
                chunk.setType(DataChunk::End);
            }

            if (c.getType() == DataChunk::End) { chunk.setType(DataChunk::End); }
            return chunk;
        }

        bool setFilt(std::string &str) {
            if (str.empty()) { return false; }
            const bool f = str[0] == 'x';
            str.erase(0, 1); // remove first char
            return f;
        }
    };

    using FilterBlockMock = GenericBlock<FilterProcessorMock>;

    bool operator==(const Pos &l, const Pos &r) {
        return l.row == r.row && l.col == r.col;
    }

    bool operator==(const Rect &l, const Rect &r) {
        return l.topLeft == r.topLeft && l.bottomRight == r.bottomRight;
    }

    Rect makeRect(Size topLeftRow, Size topLeftCol, Size bottomRightRow, Size bottomRightCol) {
        Rect r{};
        r.topLeft.row = topLeftRow;
        r.topLeft.col = topLeftCol;
        r.bottomRight.row = bottomRightRow;
        r.bottomRight.col = bottomRightCol;
        return r;
    }

    LabelData makeLabelData(Uint16 label, Size size, const Rect &rect) {
        return LabelData{label, size, rect};
    }

    // This tester runs Labelling and Computation blocks.
    // Labelling block uses input, which is set by user via pushInput().
    // Output of the computation block is saved into output_ data member.
    // After running test (via doTest()), output can be tested by checkElement()
    // and allElementsChecked() methods.
    class Tester {
    public:
        void pushInput(const std::string &str) { input_.push(str); }

        void doTest() {
            FilterBlockMock b2{input_};
            LabellingBlock b3{&b2, static_cast<Uint16>(input_.front().size())};
            last_ = std::make_unique<ComputationBlock>(&b3);
            last_->start();
            while (true) {
                ComputedChunk c{};
                last_->getProcessor().getQueue().wait_dequeue(c);
                for (auto &ld : c.labelData) {
                    output_.push_back(ld);
                }
                if (c.getType() == DataChunk::End) {
                    break;
                }
            }
            last_->stop();
        }

        bool checkElement(const LabelData &labelData) {
            auto iter = std::find_if(
                    output_.begin(), output_.end(),
                    [&](const LabelData &ld) {
                        return (labelData.label == 0 || labelData.label == ld.label) &&
                               ld.size == labelData.size &&
                               ld.rect == labelData.rect;
                    });
            if (iter == output_.end()) {
                return false;
            } else {
                output_.erase(iter);
                return true;
            }
        }

        bool allElementsChecked() const { return output_.empty(); }

    private:
        std::queue<std::string> input_;
        std::unique_ptr<ComputationBlock> last_;
        std::list<LabelData> output_;
    };

    TEST_CASE("ComputationBlock1", "[ComputationBlock]") {
        SECTION("labels reused by filter block") {
            Tester t{};
            t.pushInput("x_x_");
            t.pushInput("____");
            t.pushInput("x_x_");
            t.pushInput("____");
            t.pushInput("x_x_");
            t.pushInput("____");

            t.doTest();

            REQUIRE(t.checkElement(makeLabelData(0, 1, makeRect(0, 0, 0, 0))));
            REQUIRE(t.checkElement(makeLabelData(0, 1, makeRect(0, 2, 0, 2))));
            REQUIRE(t.checkElement(makeLabelData(0, 1, makeRect(2, 2, 2, 2))));
            REQUIRE(t.checkElement(makeLabelData(0, 1, makeRect(2, 0, 2, 0))));
            REQUIRE(t.checkElement(makeLabelData(0, 1, makeRect(4, 0, 4, 0))));
            REQUIRE(t.checkElement(makeLabelData(0, 1, makeRect(4, 2, 4, 2))));

            REQUIRE(t.allElementsChecked());
        }

        SECTION("labels on the last line") {
            Tester t{};
            t.pushInput("____");
            t.pushInput("_xx_");

            t.doTest();

            // label:1 size:2 topLeft:1,1 bottomRight:1,2
            REQUIRE(t.checkElement(makeLabelData(1, 2, makeRect(1, 1, 1, 2))));

            REQUIRE(t.allElementsChecked());
        }

        SECTION("labels merged") {
            Tester t{};
            t.pushInput("x__x");
            t.pushInput("x__x");
            t.pushInput("xxxx");

            t.doTest();

            // label:1 size:8 topLeft:0,0 bottomRight:2,3
            REQUIRE(t.checkElement(makeLabelData(1, 8, makeRect(0, 0, 2, 3))));

            REQUIRE(t.allElementsChecked());
        }

        SECTION("diagonal labels") {
            Tester t{};
            t.pushInput("x_");
            t.pushInput("_x");
            t.pushInput("x_");

            t.doTest();

            // label:1 size:3 topLeft:0,0 bottomRight:2,1
            REQUIRE(t.checkElement(makeLabelData(1, 3, makeRect(0, 0, 2, 1))));
            REQUIRE(t.allElementsChecked());
        }

        SECTION("diagonal labels 2") {
            Tester t{};
            t.pushInput("xxxxxx");
            t.pushInput("_xxxxx");
            t.pushInput("__xxxx");
            t.pushInput("x__xxx");
            t.pushInput("xx__xx");

            t.doTest();

            REQUIRE(t.checkElement(makeLabelData(1, 20, makeRect(0, 0, 4, 5))));
            REQUIRE(t.checkElement(makeLabelData(2, 3, makeRect(3, 0, 4, 1))));

            REQUIRE(t.allElementsChecked());
        }

        SECTION("chessboard labels") {
            Tester t{};
            t.pushInput("x_x_x_x_");
            t.pushInput("_x_x_x_x");
            t.pushInput("x_x_x_x_");
            t.pushInput("_x_x_x_x");
            t.pushInput("________");

            t.doTest();

            // label:1 size:16 topLeft:0,0 bottomRight:3,7
            REQUIRE(t.checkElement(makeLabelData(1, 16, makeRect(0, 0, 3, 7))));

            REQUIRE(t.allElementsChecked());
        }

        SECTION("chessboard labels 2") {
            Tester t{};
            t.pushInput("x_x_x_x_x_x_x_x_");
            t.pushInput("_x_x_x_x_x_x_x_x");
            t.pushInput("x_x_x_x_x_x_x_x_");
            t.pushInput("_x_x_x_x_x_x_x_x");
            t.pushInput("________________");

            t.doTest();

            REQUIRE(t.checkElement(makeLabelData(1, 32, makeRect(0, 0, 3, 15))));

            REQUIRE(t.allElementsChecked());
        }
    }
}