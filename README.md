# Pipeliner

This library helps to organize processing of data stream in a DSP like fashion.

Data stream is divided into DataChunks. Processing done by Blocks. Each Block
perfoms some operation on a DataChunk and then transfers processed data chunk
to a next block. All blocks work in parallel, each block works in its own
thread.

Library has predefined DataChunk, which has two uint8 elements. Also library
has predefined blocks - RandomNumberGeneratorBlock, which generates chunks with
random numbers, and FilterBlock, which filters data by some threshold value:

```cpp
// Block b2 is linked to it's previous block b1, so blocks are chained
// together: 
RandomNumberGeneratorBlock b1{};
FilterBlock b2{128, &b1};

// We start and stop last block in a chain, all other blocks started and
// stopped recursively
b2.start();
for (int i = 0; i != 100; ++i) {
    b2.waitChunk();
}
b2.stop();

```

If you need to define your own block, then make a class, derived from
BasicBlock.  If you need to add some fields to a data chunk, derive from
DataChunk class (see example dir):

```cpp 
class MyDataChunk : public DataChunk {
public:
    MyDataChunk(DataChunk::Type type) : DataChunk{type} {}

    double myDoubleValue;
    std::vector<int> myVec;
};

class MyBlock : public BasicBlock {
public:
    MyBlock(BasicBlock *previousBlock) : BasicBlock{previousBlock} {}

    std::unique_ptr<DataChunk> processChunk(std::unique_ptr<DataChunk> chunk) override {
        if (chunk->getType() == DataChunk::End) {
            return nullptr;
        }

        auto processedChunk = std::make_unique<MyDataChunk>(DataChunk::Data);
        processedChunk->myDoubleValue = chunk->data1 * chunk->data2;
        return std::move(processedChunk);
    }
};

```


## Update 

Architecture remain unchanged, but one error was fixed:

If block A processes chunks too fast, next block B has no time to process each chunk from block A
and some chunks are lost (overwritten by block A on the next iteration). 

I see two solutions to this problem: 
a) Add buffer queue for chunks, so processed chunks pushed to the queue and next block pops them 
when ready.
b) On each iteration, after chunk processing and before starting next iteration, wait for the next 
block to grab the chunk.

Case a) is faster, because block work on maximum possible speed and don't wait for the
next block. 
Case b) has lower memory requirements, because chunk buffer is not required.

I desided to implement case b) because 
* We get time determinism: we know that block processes chunk, which was processed by the previous
block exactly on the previous iteration.
* I feel it's more suitable for requirements, where nothing said about buffer queue beetwen 
blocks,
* it takes minimal changes in code.

(See commit #4f41a4f Fix error: lost chunks if block too fast)


## Future improvements

Need to add error reporting mechanism. We cannot use exceptions because blocks executed in their
own thread. Class Error could be added to the DataChunk as a member. If an error has occurred in 
the block, we must report error to the user and stop all the blocks:
* errored block issues chunk with a error, which will be forwarded by the next blocks to the user
* errored block calls stop() on previous block, which will stop all previous blocks recursively


## Update 2

Fixed error in case of 'chessboard' labels. Error could be fixed by using
explicit chained Merge struct (as suggested in the report), but I think, more
easy solution (requires less changes in the code) is to keep merge chain
implicitly, by updating neibhor labels on every merge in the
`LabellingBlock::curRow_`, `LabellingBlock::prevRow_` arrays. See commit
`#31afb2d Fix labelling error in 'chessboard' case`.

For optimisation of `mutex/condition variable` barriers between blocks, 
barriers replaced with consumer/producer lock-free queue (see 
`BasicBlock::queue_). 
Source: [readerwriterqueue](https://github.com/cameron314/readerwriterqueue)

To optimize avay `mutex` used for label reuse mechanism in the LabellingBlock 
and ComputationBlock (see `LabelSet::mutex_`), another queue was added (see
`BasicBlock::reverseQueue_`). Now bidirectional communication between blocks is
possible.

Simple benchmarking was added to the project. It measures performance of the
ComputationBlock iteration. Run `pipeliner_benchmark` executable. Benchmark 
done with Hayai (source: [hayai](https://github.com/nickbruun/hayai)).
Below is the output on my PC, when compiled with Release flag. It says,
ComputationBlock iteration takes 13.207 microseconds on average.
```
[==========] Running 1 benchmark..
[ RUN      ] PipelinerBenchmark._ (10 runs, 1000 iterations per run)
[     DONE ] PipelinerBenchmark._ (132.070881 ms)
[   RUNS   ]        Average time: 13207.088 us (~4713.162 us)
                    Fastest time: 8716.396 us (-4490.692 us / -34.002 %)
                    Slowest time: 20621.152 us (+7414.064 us / +56.137 %)
                     Median time: 11083.291 us (1st quartile: 8899.155 us | 3rd quartile: 18570.300 us)
                                  
             Average performance: 75.71692 runs/s
                Best performance: 114.72632 runs/s (+39.00940 runs/s / +51.52006 %)
               Worst performance: 48.49390 runs/s (-27.22302 runs/s / -35.95368 %)
              Median performance: 90.22591 runs/s (1st quartile: 112.37022 | 3rd quartile: 53.84943)
                                  
[ITERATIONS]        Average time: 13.207 us (~4.713 us)
                    Fastest time: 8.716 us (-4.491 us / -34.002 %)
                    Slowest time: 20.621 us (+7.414 us / +56.137 %)
                     Median time: 11.083 us (1st quartile: 8.899 us | 3rd quartile: 18.570 us)
                                  
             Average performance: 75716.91749 iterations/s
                Best performance: 114726.31578 iterations/s (+39009.39829 iterations/s / +51.52006 %)
               Worst performance: 48493.89598 iterations/s (-27223.02152 iterations/s / -35.95368 %)
              Median performance: 90225.90853 iterations/s (1st quartile: 112370.21942 | 3rd quartile: 53849.42623)
[==========] Ran 1 benchmark..
```

Profiling code (with `valgrind --tool=callgrind` under Linux) showed heavy
usage of `std::vector` in the `LabellingBlock::processElement()`, which was
used to store neibhor labels. This vector replaced with the automatic (on the
stack) array. See commit `#2e8556b Optimize std::vector in the LabellingBlock`.  

Next best candidate for the optimisation are DataChunk's malloc/free
operations. Profiler showes, that those operations take 50% of run time.

