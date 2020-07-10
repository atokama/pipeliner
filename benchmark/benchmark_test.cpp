#include <pipeliner/computation_block.h>
#include <pipeliner/generator_block.h>

#include "hayai_main.hpp"

namespace pipeliner {

    class PipelinerBenchmark : public hayai::Fixture {
    public:
        void SetUp() override {
            const auto generatorBlockDelay = 0ms;
            const int threshold = 128;

            const int width = 10;
            const int rowCount = 10;

            b1_ = std::make_unique<RandomNumberGeneratorBlock>(generatorBlockDelay);
            b2_ = std::make_unique<FilterBlock>(threshold, width + 8, b1_.get());
            b3_ = std::make_unique<LabellingBlock>(width, b2_.get());
            b4_ = std::make_unique<ComputationBlock>(b3_.get());
            b4_->start();
        }

        void TearDown() override {
            b4_->stop();
        }

        std::unique_ptr<RandomNumberGeneratorBlock> b1_;
        std::unique_ptr<FilterBlock> b2_;
        std::unique_ptr<LabellingBlock> b3_;
        std::unique_ptr<ComputationBlock> b4_;
    };

    // Quote from the Hayai documentation at https://bruun.co/2012/02/07/easy-cpp-benchmarking
    //
    // > The third and fourth parameters specify the number of runs and iterations to perform.
    // > A run is the execution of the code in the brackets performed iterations number of times.
    BENCHMARK_F(PipelinerBenchmark, _, 10, 1000) {
        b4_->waitChunk();
    }

}
