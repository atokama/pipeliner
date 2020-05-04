#include <iostream>

#include <pipeliner/generator_block.h>

using namespace pipeliner;

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

int main() {
    RandomNumberGeneratorBlock b1{};
    MyBlock b2{&b1};

    b2.start();

    for (int i = 0; i != 10; ++i) {
        auto processedChunk = b2.waitChunk();
        auto pc = dynamic_cast<MyDataChunk *>(processedChunk.get());
        std::cout << "> " << pc->myDoubleValue << std::endl;
    }

    b2.stop();
    return 0;
}