#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <atomic>

#include <pipeliner/processor.h>

namespace pipeliner {

    using LockGuard = std::lock_guard<std::mutex>;
    using UniqueLock = std::unique_lock<std::mutex>;

    struct BasicBlock {
        virtual ~BasicBlock() = default;

        BasicBlock(BasicBlock *prevBlock) : prevBlock_{prevBlock} {}

        void start() {
            thread_ = std::make_unique<std::thread>([this]() { doWork(); });
        }

        void stop() {
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

        virtual void doWork() = 0;

        BasicBlock *prevBlock_{nullptr};
        std::atomic_bool shouldStop_{false};

    private:
        std::unique_ptr<std::thread> thread_;
        mutable std::mutex joinMutex_;
    };

    template<typename ProcessorType>
    struct GenericBlock : BasicBlock {
        template<typename PrevProcessorType, typename... Args>
        GenericBlock(GenericBlock<PrevProcessorType> *prevBlock, Args &&... args)
                : BasicBlock{prevBlock},
                  proc_{std::forward<Args>(args)...} {
            if (prevBlock) {
                auto &q1 = prevBlock->getProcessor().getQueue();
                auto &q2 = prevBlock->getProcessor().getReverseQueue();
                proc_.setPrevQueue(q1);
                proc_.setPrevReverseQueue(q2);
            }
        }

        template<typename... Args>
        GenericBlock(Args &&... args)
                : BasicBlock{nullptr},
                  proc_{std::forward<Args>(args)...} {
        }

        ProcessorType &getProcessor() { return proc_; }

        void doWork() override {
            if (prevBlock_) { prevBlock_->start(); }

            while (true) {
                proc_.doReverseIteration();
                const bool stop{shouldStop_};

                if (!proc_.doProcessIteration(stop)) { break; }
            }
        }

    private:
        ProcessorType proc_;
    };

}

