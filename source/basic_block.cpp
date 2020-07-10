#include <pipeliner/basic_block.h>


namespace pipeliner {

    using LockGuard = std::lock_guard<std::mutex>;
    using UniqueLock = std::unique_lock<std::mutex>;

    void BasicBlock::start() {
        thread_ = std::make_unique<std::thread>([this]() { doWork(); });
    }

    void BasicBlock::doWork() {
        if (prevBlock_) {
            prevBlock_->start();
        }

        while (true) {
            std::unique_ptr<DataChunk> chunk{nullptr};
            bool endChunk{false};

            if (prevBlock_) {
                chunk = prevBlock_->waitChunk();
                if (chunk && chunk->getType() == DataChunk::End) {
                    endChunk = true;
                    shouldStop_ = true;
                }
            }

            if (auto c = processChunk(std::move(chunk))) {
                if (c->getType() == DataChunk::End) {
                    shouldStop_ = true;
                }
                queue_.enqueue(std::move(c));
            }

            while (reverseQueue_.try_dequeue(chunk)) {
                processReverseChunk(std::move(chunk));
            }

            if (shouldStop_) {
                if (!endChunk) {
                    queue_.enqueue(
                            std::make_unique<DataChunk>(DataChunk::End));
                }
                break;
            }
        }
    }

    std::unique_ptr<DataChunk> BasicBlock::waitChunk() {
        std::unique_ptr<DataChunk> c{nullptr};
        queue_.wait_dequeue(c);
        return std::move(c);
    }

    void BasicBlock::enqueueReverseChunk(std::unique_ptr<DataChunk> chunk) {
        if (prevBlock_) {
            prevBlock_->reverseQueue_.enqueue(std::move(chunk));
        }
    }

    void BasicBlock::stop() {
        if (prevBlock_) {
            prevBlock_->stop();
        } else {
            shouldStop_ = true;
        }

        if (thread_) {
            LockGuard lock{joinMutex_};
            if (thread_->joinable()) { thread_->join(); }
        }
    }

}