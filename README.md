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
