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

    struct TesterProcessor : GenericProcessor<TesterProcessor, TesterChunk, TesterChunk> {
        TesterChunk processChunkImpl(TesterChunk c) {
            if (c.getType() == DataChunk::Empty) {
                c.setType(DataChunk::Data);
            }
            ++c.value;
            return std::move(c);
        }
    };

    using BasicBlockTester = GenericBlock<TesterProcessor>;


    TEST_CASE("BasicBlock", "[BasicBlock]") {
        BasicBlockTester g{};
        BasicBlockTester generator{&g};

        generator.start();

        auto t = ns();
        const auto tFinish = 300000 + t;
        while (ns() < tFinish) {
            TesterChunk c{};
            generator.getProcessor().getQueue().wait_dequeue(c);
            std::cout << "iteration time ns: " << ns() - t << std::endl;
            t = ns();
        }

        generator.stop();
    }

}
