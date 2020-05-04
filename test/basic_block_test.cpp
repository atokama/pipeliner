#include <pipeliner/basic_block.h>

#include <iostream>
#include <cstdint>

#include <spdlog/spdlog.h>
#include <timer.h>

#include "catch.hpp"

namespace pipeliner {

    struct TesterChunk : public DataChunk {
        TesterChunk(DataChunk::Type type) : DataChunk{type}, value{} {}

        Uint8 value;
    };

    class BasicBlockTester : public BasicBlock {
    public:
        BasicBlockTester(BasicBlock *prevBlock = nullptr) : BasicBlock{prevBlock} {}

        std::unique_ptr<DataChunk> processChunk(std::unique_ptr<DataChunk> chunk) override {
            int v = 0;
            if (chunk && chunk->getType() == DataChunk::Data) {
                auto testerChunk = dynamic_cast<TesterChunk *>(chunk.get());
                v = testerChunk->value + 1;
            }
            auto processedChunk = std::make_unique<TesterChunk>(
                    v == 100 ? DataChunk::End : DataChunk::Data);

            processedChunk->value = v;
            return std::move(processedChunk);
        };
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
