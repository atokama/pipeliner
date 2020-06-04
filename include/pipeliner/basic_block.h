#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdint>

#include <pipeliner/debug.h>

namespace pipeliner {

    using Uint8 = std::uint8_t;

    class DataChunk {
    public:
        enum Type {
            Data, End,
        };

        virtual ~DataChunk() = default;

        DataChunk(DataChunk::Type type = Data) : type_{type}, data1{0}, data2{0} {}

        Type getType() const { return type_; }

        Uint8 data1, data2;

    private:
        const Type type_;
    };

    class BasicBlock {
    public:
        virtual ~BasicBlock() = default;

        BasicBlock(BasicBlock *prevBlock = nullptr) : prevBlock_{prevBlock} {}

        void start();

        std::unique_ptr<DataChunk> waitChunk();

        void stop();

        virtual std::unique_ptr<DataChunk> processChunk(std::unique_ptr<DataChunk> chunk) = 0;

        Debug &debug() { return debug_; }

    private:
        BasicBlock *const prevBlock_;
        std::unique_ptr<DataChunk> chunk_;
        bool shouldStop_{false};
        std::unique_ptr<std::thread> thread_;
        std::condition_variable cv_;
        mutable std::mutex mutex_, joinMutex_;
        Debug debug_;
    };

}

