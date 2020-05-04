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
together: RandomNumberGeneratorBlock b1{}; FilterBlock b2{128, &b1};

// We start and stop last block in a chain, all other blocks started and
stopped recursively
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
