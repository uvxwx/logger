#pragma once
#include <string>
#include <string_view>
#include <type_traits>

template <typename T> class Metric {
public:
    T combine(T rhs) const { return static_cast<T &>(*this).combine(rhs); }

    std::string formattedValue() const { return static_cast<T &>(*this).formattedValue(); }
};

class HttpRequestsRps : public Metric<HttpRequestsRps> {
private:
    int64_t value;

public:
    static constexpr std::string_view kName = "HTTP requests RPS";
    static constexpr int64_t kInitial = 0;
    HttpRequestsRps(int64_t initial) : value(initial) {}
    HttpRequestsRps() : value(kInitial) {}

    HttpRequestsRps combine(HttpRequestsRps rhs) const
    {
        return HttpRequestsRps(value + rhs.value);
    }

    std::string formattedValue() const { return std::to_string(value); }
};

class Product : public Metric<Product> {
private:
    double value;

public:
    static constexpr std::string_view kName = "product";
    static constexpr double kInitial = 1.0;
    Product(double initial) : value(initial) {}
    Product() : value(kInitial) {}

    Product combine(Product rhs) const { return Product(value * rhs.value); }

    std::string formattedValue() const { return std::to_string(value); }
};

class AverageResponseTime : public Metric<AverageResponseTime> {
private:
    int64_t sumTimes;
    int64_t responses;

public:
    static constexpr std::string_view kName = "Average response time";
    static constexpr int64_t kInitialSumTimes = 0;
    static constexpr int64_t kInitialResponses = 0;
    AverageResponseTime(int64_t initialSumTimes, int64_t initialResponses)
        : sumTimes(initialSumTimes), responses(initialResponses)
    {
    }
    AverageResponseTime() : sumTimes(kInitialSumTimes), responses(kInitialResponses) {}

    AverageResponseTime combine(AverageResponseTime rhs) const
    {
        return AverageResponseTime(sumTimes + rhs.sumTimes, responses + rhs.responses);
    }

    std::string formattedValue() const
    {
        return std::to_string(static_cast<double>(sumTimes) / static_cast<double>(responses));
    }
};
