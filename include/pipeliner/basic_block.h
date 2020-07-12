#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <atomic>

#include <readerwriterqueue/atomicops.h>
#include <readerwriterqueue/readerwriterqueue.h>

#include <pipeliner/debug.h>

namespace pipeliner {

    using Uint8 = std::uint8_t;

    class DataChunk {
    public:
        enum Type {
            Data, Empty, End,
        };

        virtual ~DataChunk() = default;

        DataChunk(DataChunk::Type type = Data) : type_{type}, data1{0}, data2{0} {}

        Type getType() const { return type_; }

        void setType(Type type) { type_ = type; }

        Uint8 data1, data2;

    private:
        Type type_;
    };

    class BasicBlock {
    public:
        virtual ~BasicBlock() = default;

        BasicBlock(BasicBlock *prevBlock = nullptr) : prevBlock_{prevBlock} {}

        void start();

//        std::unique_ptr<DataChunk> waitChunk();

        void stop();

        Debug &debug() { return debug_; }

    protected:
        virtual bool processChunk(bool shouldStop) = 0;

        virtual void processReverseChunk() {}

//        void enqueueReverseChunk(std::unique_ptr<DataChunk> chunk);

        BasicBlock *const prevBlock_{nullptr};
    private:
        void doWork();

        std::atomic_bool shouldStop_{false};
        std::unique_ptr<std::thread> thread_;
        mutable std::mutex joinMutex_;
        Debug debug_;
//        moodycamel::BlockingReaderWriterQueue<
//                std::unique_ptr<DataChunk>> queue_, reverseQueue_;
    };

}

