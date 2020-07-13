#pragma once

#include <readerwriterqueue/atomicops.h>
#include <readerwriterqueue/readerwriterqueue.h>

#include <pipeliner/debug.h>

namespace pipeliner {

    using Uint8 = std::uint8_t;

    template<typename ChunkType>
    using Queue = moodycamel::BlockingReaderWriterQueue<ChunkType>;

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

    class BasicProcessor {
    public:
        virtual ~BasicProcessor() = default;

        virtual bool doProcessIteration(bool shouldStop) = 0;

        virtual void doReverseIteration() {}
    };

    template<typename Derived, typename ChunkType, typename PrevChunkType>
    class GenericProcessor : public BasicProcessor {
    public:
        ChunkType processChunk(PrevChunkType prevChunk) {
            return static_cast<Derived *>(this)->processChunkImpl(std::move(prevChunk));
        }

        void processReverseChunk(ChunkType c) {
            return static_cast<Derived *>(this)->processReverseChunkImpl(std::move(c));
        }

        bool doProcessIteration(bool shouldStop) override {
            PrevChunkType c{DataChunk::Empty};
            if (prevQueue_) {
                prevQueue_->wait_dequeue(c);
            }
            if (shouldStop) {
                c.setType(DataChunk::End);
            }

            auto c1 = processChunk(std::move(c));

            if (shouldStop || c1.getType() == DataChunk::End) {
                shouldStop = true;
                c1.setType(DataChunk::End);
            }
            if (c1.getType() != DataChunk::Empty) {
                queue_.enqueue(std::move(c1));
            }
            return !shouldStop;
        }

        void doReverseIteration() {
            ChunkType c{};
            while (reverseQueue_.try_dequeue(c)) {
                processReverseChunk(std::move(c));
            }
        }

        void enqueueReverseChunk(PrevChunkType c) {
            if (prevReverseQueue_) {
                prevReverseQueue_->enqueue(std::move(c));
            }
        }

        Queue<ChunkType> &getQueue() { return queue_; }

        Queue<ChunkType> &getReverseQueue() { return reverseQueue_; };

        void setPrevReverseQueue(Queue <PrevChunkType> &prevReverseQueue) { prevReverseQueue_ = &prevReverseQueue; }

        void setPrevQueue(Queue <PrevChunkType> &prevQueue) { prevQueue_ = &prevQueue; }

        Debug &debug() { return debug_; }

    protected:
        ChunkType processChunkImpl(PrevChunkType prevChunk) { return {}; }

        void processReverseChunkImpl(ChunkType c) {}

    private:
        Queue<ChunkType> queue_, reverseQueue_;
        Queue<PrevChunkType> *prevQueue_{nullptr}, *prevReverseQueue_{nullptr};
        Debug debug_;
    };

}