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
            processReverseChunk();

            const bool stop{shouldStop_};
            const bool shouldContinue = processChunk(stop);
            if (!shouldContinue) {
                break;
            }
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