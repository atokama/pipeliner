#include <pipeliner/computation_block.h>
#include <pipeliner/generator_block.h>

#include "hayai_main.hpp"

namespace pipeliner {

    // Quote from the Hayai documentation at https://bruun.co/2012/02/07/easy-cpp-benchmarking
    //
    // > The third and fourth parameters specify the number of runs and iterations to perform.
    // > A run is the execution of the code in the brackets performed iterations number of times.
    //
    // To calcutate time to process one chunk use formula:
    // T = AverageTime / rowCount / width * 2
    //     ^                                ^
    //     from benchmark output            two elements per chunk

    BENCHMARK(AllBlocks, RandomNumberGenerator, 10, 1) {
        const auto generatorBlockDelay = 0ms;
        const int threshold = 128;

        const int width = 10;
        const int rowCount = 10;

        RandomNumberGeneratorBlock b1{generatorBlockDelay};
        FilterBlock b2{threshold, width + 8, &b1};
        LabellingBlock b3{width, &b2};
        ComputationBlock b4{&b3};

        b4.start();
        for (int i{0}; i != width * rowCount; ++i) { b4.waitChunk(); }
        b4.stop();
    }

}
