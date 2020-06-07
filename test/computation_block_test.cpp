#include <pipeliner/computation_block.h>
#include <pipeliner/generator_block.h>

#include <iostream>

#include "catch.hpp"

namespace pipeliner {

    TEST_CASE("ComputationBlock", "[ComputationBlock]") {

        const auto generatorBlockDelay = 0ms;
        const int threshold = 128;
        const int width = 32;

        RandomNumberGeneratorBlock b1{generatorBlockDelay};
        FilterBlock b2{threshold, width, &b1};
        LabellingBlock b3{width - 8, &b2};
        ComputationBlock b4{&b3};

        b4.start();

        for (int i = 0; i != 10; ++i) {
            b4.waitChunk();
        }

        b4.stop();

        std::string l2;
        do {
            l2 = b2.debug().popLine();
            std::cout << l2 << std::endl;
        } while (l2.size() != 0);

        std::cout << std::endl;

        std::string l4;
        do {
            l4 = b4.debug().popLine();
            std::cout << l4 << std::endl;
        } while (l4.size() != 0);
    }

    class FilterBlockMock : public BasicBlock {
    public:
        FilterBlockMock(const std::queue<std::string> &data) : BasicBlock{nullptr}, data_{data} {}

        std::unique_ptr<DataChunk> processChunk(std::unique_ptr<DataChunk>) override {
            if (data_.empty()) { return std::make_unique<DataChunk>(DataChunk::End); }

            auto ch = std::make_unique<FilteredChunk>();
            ch->filt1 = setFilt(data_.front());
            ch->filt2 = setFilt(data_.front());

            if (data_.front().empty()) { data_.pop(); }

            return std::move(ch);
        }

    private:
        bool setFilt(std::string &str) {
            if (str.empty()) { return false; }
            const bool f = str[0] == 'x';
            str.erase(0, 1); // remove first char
            return f;
        }

        std::queue<std::string> data_;
    };

    class Tester {
    public:
        void pushInput(const std::string &str) { input_.push(str); }

        void doTest() {
            FilterBlockMock b2{input_};
            LabellingBlock b3{input_.front().size(), &b2};
            last_ = std::make_unique<ComputationBlock>(&b3);
            last_->start();
            while (true) {
                auto ch = last_->waitChunk();
                if (ch->getType() == DataChunk::End) { break; }
            }
            last_->stop();
        }

        std::string popOutput() { return last_->debug().popLine(); }

        std::queue<std::string> input_;
        std::unique_ptr<BasicBlock> last_;
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
            REQUIRE("label:1 size:1 topLeft:0,0 bottomRight:0,0" == t.popOutput());
            REQUIRE("label:2 size:1 topLeft:0,2 bottomRight:0,2" == t.popOutput());
            REQUIRE("label:1 size:1 topLeft:2,2 bottomRight:2,2" == t.popOutput());
            REQUIRE("label:3 size:1 topLeft:2,0 bottomRight:2,0" == t.popOutput());
            REQUIRE("label:1 size:1 topLeft:4,2 bottomRight:4,2" == t.popOutput());
            REQUIRE("label:2 size:1 topLeft:4,0 bottomRight:4,0" == t.popOutput());
        }

        SECTION("labels on the last line") {
            Tester t{};
            t.pushInput("____");
            t.pushInput("_xx_");

            t.doTest();
            REQUIRE("label:1 size:2 topLeft:1,1 bottomRight:1,2" == t.popOutput());
        }

        SECTION("labels merged") {
            Tester t{};
            t.pushInput("x__x");
            t.pushInput("x__x");
            t.pushInput("xxxx");

            t.doTest();
            REQUIRE("label:1 size:8 topLeft:0,0 bottomRight:2,3" == t.popOutput());
        }

        SECTION("diagonal labels") {
            Tester t{};
            t.pushInput("x_");
            t.pushInput("_x");
            t.pushInput("x_");

            t.doTest();
            REQUIRE("label:1 size:3 topLeft:0,0 bottomRight:2,1" == t.popOutput());
        }

        SECTION("diagonal labels 2") {
            Tester t{};
            t.pushInput("xxxxxx");
            t.pushInput("_xxxxx");
            t.pushInput("__xxxx");
            t.pushInput("x__xxx");
            t.pushInput("xx__xx");

            t.doTest();
            REQUIRE("label:1 size:20 topLeft:0,0 bottomRight:4,5" == t.popOutput());
            REQUIRE("label:2 size:3 topLeft:3,0 bottomRight:4,1" == t.popOutput());
        }
    }
}