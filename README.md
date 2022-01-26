# Pipeliner

Pipeliner is a library for processing stream of data, DSP like. Incoming data
split to chunks and processed in separate threads, using fast queues to connect
processing blocks.

Class Block is a single processing unit. Blocks linked to each other forming
Pipeline. Blocks can have different input and output data types, known at
compile time.

Example of processing sound from microphone:

```cpp

// User-defined processing blocks
//
class Acquision : public Block< Acquision > { ... };
class Filter1 : public Block< Filter1 > { ... };
class Filter2 : public Block< Filter2 > { ... };

int main() {

    // Setup pipeline
    //
    Acquision block_1{ Mic::Device{ 16, Mic::PCM } };
    Filter1 block_2{ 128, &block_1 };
    Filter2 block_3{ "abc",
    	std::vector< std::vector< float > >, &block_2 };

    // Start pipeline through the last block
    //
    block_3.start();

    // Main loop
    //
    for (;;) {
        auto processed_chunk = block_3.waitChunk();
        play_sound( processed_chunk );
    }

    // Stop pipeline
    //
    block_3.stop();
}
```

## Benchmark

```
[==========] Running 1 benchmark..
[ RUN      ] PipelinerBenchmark._ (10 runs, 100000 iterations per run)
[     DONE ] PipelinerBenchmark._ (480.983284 ms)
[   RUNS   ]        Average time: 48098.328 us (~5396.922 us)
                    Fastest time: 40612.542 us (-7485.786 us / -15.564 %)
                    Slowest time: 55509.178 us (+7410.850 us / +15.408 %)
                     Median time: 48351.056 us (1st quartile: 44692.788 us | 3rd quartile: 52244.256 us)

             Average performance: 20.79074 runs/s
                Best performance: 24.62294 runs/s (+3.83219 runs/s / +18.43220 %)
               Worst performance: 18.01504 runs/s (-2.77570 runs/s / -13.35067 %)
              Median performance: 20.68207 runs/s (1st quartile: 22.37497 | 3rd quartile: 19.14086)

[ITERATIONS]        Average time: 0.481 us (~0.054 us)
                    Fastest time: 0.406 us (-0.075 us / -15.564 %)
                    Slowest time: 0.555 us (+0.074 us / +15.408 %)
                     Median time: 0.484 us (1st quartile: 0.447 us | 3rd quartile: 0.522 us)

             Average performance: 2079074.33224 iterations/s
                Best performance: 2462293.54469 iterations/s (+383219.21245 iterations/s / +18.43220 %)
               Worst performance: 1801503.88824 iterations/s (-277570.44400 iterations/s / -13.35067 %)
              Median performance: 2068207.17699 iterations/s (1st quartile: 2237497.46827 | 3rd quartile: 1914086.01933)
[==========] Ran 1 benchmark..
```


## Refactoring

`Processor` s a generic base class now, implemented with CRTP pattern to
achieve static polymorphism.

[CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)


# Related papers

[Concurrent Threaded Pipeline architechture](https://liberty.princeton.edu/Publications/cutr07_ff.pdf)


## TODO

Add error reporting mechanism. If error occurred in the block, stop processing
and propage error through the pipeline.


