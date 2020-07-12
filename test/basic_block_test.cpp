#include <pipeliner/basic_block.h>

#include <iostream>
#include <cstdint>

#include <spdlog/spdlog.h>
#include <timer.h>

#include "catch.hpp"

namespace pipeliner {

    struct TesterChunk : public DataChunk {
        TesterChunk(DataChunk::Type type = DataChunk::Data) : DataChunk{type}, value{} {}

        Uint8 value;
    };

    class BasicBlockTester : public BasicBlock {
    public:
        BasicBlockTester(BasicBlock *prevBlock = nullptr) : BasicBlock{prevBlock} {}

        TesterChunk waitChunk() {
            TesterChunk c{};
            queue_.wait_dequeue(c);
            return std::move(c);
        }

        bool processChunk(bool shouldStop) override {
            int v = 0;
            if (prevBlock_) {
                auto chunk = dynamic_cast<BasicBlockTester *>(prevBlock_)->waitChunk();
                if (chunk.getType() == DataChunk::End) {
                    shouldStop = true;
                } else {
                    v = chunk.value + 1;
                }
            }

            if (v == 100) {
                shouldStop = true;
            }

            TesterChunk processedChunk{};
            processedChunk.value = v;

            if (shouldStop) {
                processedChunk.setType(DataChunk::End);
            }

            queue_.enqueue(std::move(processedChunk));

            return !shouldStop;
        };

    private:
        moodycamel::BlockingReaderWriterQueue<TesterChunk> queue_;
    };

    TEST_CASE("BasicBlock", "[BasicBlock]") {
        BasicBlockTester g{};
        BasicBlockTester generator{&g};
        generator.start();

        auto t = ns();
        const auto tFinish = 300000 + t;
        while (ns() < tFinish) {
            auto chunk = generator.waitChunk();
            std::cout << "iteration time ns: " << ns() - t << std::endl;
            t = ns();
        }

        generator.stop();
    }

}
