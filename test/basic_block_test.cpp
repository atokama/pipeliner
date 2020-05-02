#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdint>

#include <spdlog/spdlog.h>

#include "catch.hpp"

using uint8 = std::uint8_t;
using Clock = std::chrono::high_resolution_clock;
template<typename Ratio> using Duration = std::chrono::duration<double, Ratio>;
using LockGuard = std::lock_guard<std::mutex>;
using UniqueLock = std::unique_lock<std::mutex>;

class DataChunk {
public:
    enum Type {
        Data, End,
    };

    virtual ~DataChunk() = default;

    DataChunk(DataChunk::Type type = Data) : type_{type}, data1{0}, data2{0} {}

    Type getType() const { return type_; }

    uint8 data1, data2;

private:
    const Type type_;
};

class BasicBlock {
public:
    virtual ~BasicBlock() = default;

    BasicBlock(BasicBlock *prevBlock = nullptr) : prevBlock_{prevBlock} {
    }

    void start() {
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

    std::unique_ptr<DataChunk> waitChunk() {
        UniqueLock lock{mutex_};
        cv_.wait(lock, [this] { return chunk_ != nullptr; });
        auto c = std::move(chunk_);
        chunk_ = nullptr;
        return std::move(c);
    }

    void stop() {
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

    int lostChunksCount() const {
        LockGuard lock{mutex_};
        return lostChunks_;
    }

private:
    virtual std::unique_ptr<DataChunk> processChunk(std::unique_ptr<DataChunk> chunk) {
        int v = 0;
        if (chunk && chunk->getType() == DataChunk::Data) {
            v = ++chunk->data1;
        }
        auto processedChunk = std::make_unique<DataChunk>(
                v == 100 ? DataChunk::End : DataChunk::Data);

        processedChunk->data1 = v;
        return std::move(processedChunk);
    };

    BasicBlock *const prevBlock_;
    std::unique_ptr<DataChunk> chunk_;
    int lostChunks_{0};
    bool shouldStop_{false};
    std::unique_ptr<std::thread> thread_;
    std::condition_variable cv_;
    mutable std::mutex mutex_, joinMutex_;
};

TEST_CASE("BasicBlock", "[BasicBlock]") {
    BasicBlock g{};
    BasicBlock generator{&g};
    generator.start();

    auto t = Clock::now();
    const auto tFinish = Duration<std::nano>{3e5} + t;
    while (Clock::now() < tFinish) {
        auto chunk = generator.waitChunk();
        std::cout << "iteration time ns: "
                  << Duration<std::nano>{Clock::now() - t}.count()
                  << std::endl;
        t = Clock::now();
    }

    generator.stop();
}

