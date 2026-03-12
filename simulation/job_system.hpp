#pragma once

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace evo::sim {

class ThreadPool {
public:
  explicit ThreadPool(std::size_t thread_count = std::thread::hardware_concurrency()) {
    const std::size_t count = std::max<std::size_t>(1, thread_count);
    workers_.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
      workers_.emplace_back([this]() {
        for (;;) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]() { return stopping_ || !tasks_.empty(); });
            if (stopping_ && tasks_.empty()) {
              return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
          }
          task();
        }
      });
    }
  }

  ~ThreadPool() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopping_ = true;
    }
    cv_.notify_all();
    for (auto& worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  template <typename Fn>
  auto enqueue(Fn&& fn) -> std::future<std::invoke_result_t<Fn>> {
    using ReturnType = std::invoke_result_t<Fn>;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<Fn>(fn));
    std::future<ReturnType> result = task->get_future();
    {
      std::lock_guard<std::mutex> lock(mutex_);
      tasks_.emplace([task]() { (*task)(); });
    }
    cv_.notify_one();
    return result;
  }

  template <typename Fn>
  void parallel_for(std::size_t begin, std::size_t end, Fn&& fn) {
    if (end <= begin) {
      return;
    }
    const std::size_t count = end - begin;
    const std::size_t workers = std::max<std::size_t>(1, workers_.size());
    if (workers == 1 || count < workers * 2) {
      for (std::size_t i = begin; i < end; ++i) {
        fn(i);
      }
      return;
    }

    const std::size_t batch = (count + workers - 1) / workers;
    std::vector<std::future<void>> futures;
    for (std::size_t start = begin; start < end; start += batch) {
      const std::size_t stop = std::min(end, start + batch);
      futures.push_back(enqueue([start, stop, &fn]() {
        for (std::size_t i = start; i < stop; ++i) {
          fn(i);
        }
      }));
    }

    for (auto& future : futures) {
      future.get();
    }
  }

  [[nodiscard]] std::size_t thread_count() const noexcept {
    return workers_.size();
  }

private:
  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;
  std::mutex mutex_;
  std::condition_variable cv_;
  bool stopping_ {false};
};

} // namespace evo::sim
