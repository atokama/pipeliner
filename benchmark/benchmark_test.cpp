#include <pipeliner/computation_block.h>
#include <pipeliner/generator_block.h>

#include "hayai_main.hpp"

namespace pipeliner {

    class PipelinerBenchmark : public hayai::Fixture {
    public:
        void SetUp() override {
            const auto generatorBlockDelay = 0ms;
            const double threshold = 128;

            const Size width = 10;
            const Size rowCount = 10;

            b1_ = std::make_unique<GeneratorBlock>(generatorBlockDelay);
            b2_ = std::make_unique<FilterBlock>(b1_.get(), threshold, width + 8);
            b3_ = std::make_unique<LabellingBlock>(b2_.get(), static_cast<Uint16>(width));
            b4_ = std::make_unique<ComputationBlock>(b3_.get());
            b4_->start();
        }

        void TearDown() override {
            b4_->stop();
        }

        std::unique_ptr<GeneratorBlock> b1_;
        std::unique_ptr<FilterBlock> b2_;
        std::unique_ptr<LabellingBlock> b3_;
        std::unique_ptr<ComputationBlock> b4_;
    };

    // Quote from the Hayai documentation at https://bruun.co/2012/02/07/easy-cpp-benchmarking
    //
    // > The third and fourth parameters specify the number of runs and iterations to perform.
    // > A run is the execution of the code in the brackets performed iterations number of times.
    BENCHMARK_F(PipelinerBenchmark, _, 10, 100000) {
        ComputedChunk c{};
        b4_->getProcessor().getQueue().wait_dequeue(c);
    }

}
