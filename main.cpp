#include "logger.hpp"
#include "metrics.hpp"
#include <vector>

int main()
{
    Logger<HttpRequestsRps, Product, AverageResponseTime> logger("test.log");
    const size_t n = 20;
    std::vector<std::thread> threads;
    for (size_t i = 0; i < n; i++) {
        threads.push_back(std::thread([&logger]() -> void { logger.log(HttpRequestsRps(3)); }));
        threads.push_back(std::thread([&logger]() -> void { logger.log(Product(2)); }));
        threads.push_back(std::thread([&logger, i]() -> void {
            logger.log(AverageResponseTime(static_cast<int64_t>(i), static_cast<int64_t>(i) + 3));
        }));
    }
    for (auto &cur : threads) {
        if (cur.joinable())
            cur.join();
    }
    return 0;
}
