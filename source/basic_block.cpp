#include <pipeliner/basic_block.h>


namespace pipeliner {

    using LockGuard = std::lock_guard<std::mutex>;
    using UniqueLock = std::unique_lock<std::mutex>;

    void BasicBlock::start() {
        thread_ = std::make_unique<std::thread>([this]() {
            if (prevBlock_) { prevBlock_->start(); }

            while (true) {
                std::unique_ptr<DataChunk> chunk{nullptr};
                bool shouldStop = false;
                if (prevBlock_) {
                    chunk = prevBlock_->waitChunk();
                    if (!chunk) {
                        throw std::runtime_error{"Block has not returned chunk"};
                    }
                    shouldStop = chunk->getType() == DataChunk::End;
                }

                auto processedChunk = processChunk(std::move(chunk));

                {
                    LockGuard lock{mutex_};

                    shouldStop = shouldStop || shouldStop_;
                    if (shouldStop) {
                        chunk_ = std::make_unique<DataChunk>(DataChunk::End);
                    } else {
                        if (chunk_) { lostChunks_++; }
                        chunk_ = std::move(processedChunk);
                    }
                }

                cv_.notify_one();

                if (shouldStop) { break; }
            }

        });
    }

    std::unique_ptr<DataChunk> BasicBlock::waitChunk() {
        UniqueLock lock{mutex_};
        cv_.wait(lock, [this] { return chunk_ != nullptr; });
        auto c = std::move(chunk_);
        chunk_ = nullptr;
        return std::move(c);
    }

    void BasicBlock::stop() {
        if (thread_) {
            {
                LockGuard lock{mutex_};
                shouldStop_ = true;
            }
            LockGuard lock{joinMutex_};
            if (thread_->joinable()) { thread_->join(); }
        }

        if (prevBlock_) { prevBlock_->stop(); }
    }

    int BasicBlock::lostChunksCount() const {
        LockGuard lock{mutex_};
        return lostChunks_;
    }

}