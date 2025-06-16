#pragma once
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <string_view>
#include <thread>

#include "metrics.hpp"

namespace fs = std::filesystem;

template <class T, class Tuple> struct Index;

template <class T, class... Types> struct Index<T, std::tuple<T, Types...>> {
    static const size_t value = 0;
};

template <class T, class U, class... Types> struct Index<T, std::tuple<U, Types...>> {
    static const size_t value = 1 + Index<T, std::tuple<Types...>>::value;
};

template <typename... Ts> constexpr bool always_false_v = false;
template <class Func, class Tuple, size_t N = 0> void runtimeGet(Func func, Tuple &tup, size_t idx)
{
    if (N == idx) {
        std::invoke(func, std::get<N>(tup));
        return;
    }

    if constexpr (N + 1 < std::tuple_size_v<Tuple>) {
        return runtimeGet<Func, Tuple, N + 1>(func, tup, idx);
    }
}

template <typename... Ts> class Logger {
private:
    std::condition_variable cv;
    std::mutex mutex;
    std::tuple<Ts...> metrics;
    std::set<size_t> loggedIndices;
    std::ofstream out;
    bool stop = false;
    std::thread fileWriter;

public:
    Logger(fs::path path)
    {
        out.open(path, std::ios::binary | std::ios::app);
        if (!out)
            throw std::runtime_error("logger: could not open: " + path.string());
        fileWriter = std::thread([this]() -> void {
            std::unique_lock<std::mutex> lock(mutex);
            while (true) {
                cv.wait(lock, [this]() -> bool { return stop || !loggedIndices.empty(); });
                if (!loggedIndices.empty()) {
                    out << std::chrono::system_clock::now() << " UTC";
                    for (auto ind : loggedIndices) {
                        std::string str;
                        runtimeGet(
                            [&str]<typename T>(T &metric) -> void {
                                str = "\"" + std::string(metric.kName) + "\" " +
                                      metric.formattedValue();
                                metric = T();
                            },
                            metrics, ind
                        );
                        out << " " << str;
                    }
                    out << "\n";
                    loggedIndices.clear();
                }
                if (stop && loggedIndices.empty()) {
                    out.flush();
                    break;
                }
            }
        });
    }
    ~Logger()
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            stop = true;
        }
        cv.notify_one();
        if (fileWriter.joinable())
            fileWriter.join();
    }

    template <typename T> void log(T update)
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (stop)
            return;
        auto &metric = std::get<T>(metrics);
        metric = metric.combine(update);
        constexpr auto ind = Index<T, decltype(metrics)>::value;
        loggedIndices.insert(ind);
        cv.notify_one();
    }
};
