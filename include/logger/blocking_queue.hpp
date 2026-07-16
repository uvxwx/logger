#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

namespace logger {

// Простейшая блокирующая FIFO-очередь без состояния закрытия.
template <class T>
class BlockingQueue {
 public:
  // Добавляет элемент в хвост очереди.
  void Push(T value) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(std::move(value));
    }

    cv_.notify_one();
  }

  // Ждет появления элемента и возвращает его.
  T WaitPop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !queue_.empty(); });

    T value = std::move(queue_.front());
    queue_.pop();
    return value;
  }

 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

}  // namespace logger
