#include <pipeliner/basic_block.h>

#include <iostream>
#include <cstdint>

#include <spdlog/spdlog.h>

#include "catch.hpp"

namespace pipeliner {

    using uint8 = std::uint8_t;
    using Clock = std::chrono::high_resolution_clock;
    template<typename Ratio> using Duration = std::chrono::duration<double, Ratio>;

    struct TesterChunk : public DataChunk {
        TesterChunk(DataChunk::Type type) : DataChunk{type}, value{} {}

        uint8 value;
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

        auto t = Clock::now();
        const auto tFinish = Duration<std::nano>{3e5} + t;
        while (Clock::now() < tFinish) {
            auto chunk = generator.waitChunk();
            std::cout << "iteration time ns: "
                      << Duration<std::nano>{Clock::now() - t}.count()
                      << std::endl;
            t = Clock::now();
        }

        generator.stop();
    }

}
