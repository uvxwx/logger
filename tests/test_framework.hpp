#pragma once

#include <exception>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace test {

// Исключение, которым тестовые проверки останавливают текущий тест.
class Failure : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

struct TestCase {
  std::string name;
  std::function<void()> function;
};

// Возвращает общий реестр тестов.
inline std::vector<TestCase>& Registry() {
  static std::vector<TestCase> tests;
  return tests;
}

// Добавляет тест в общий реестр.
inline void RegisterTest(std::string name, std::function<void()> function) {
  Registry().push_back({std::move(name), std::move(function)});
}

// Проверяет равенство двух значений.
template <typename Left, typename Right>
void ExpectEqual(const Left& left, const Right& right, const char* left_expr,
                 const char* right_expr) {
  if (!(left == right)) {
    std::ostringstream stream;
    stream << "expected equality: " << left_expr << " == " << right_expr;
    throw Failure{stream.str()};
  }
}

// Проверяет истинность выражения.
inline void ExpectTrue(const bool condition, const char* expression) {
  if (!condition) {
    throw Failure{std::string{"expected true: "} + expression};
  }
}

// Проверяет ложность выражения.
inline void ExpectFalse(const bool condition, const char* expression) {
  if (condition) {
    throw Failure{std::string{"expected false: "} + expression};
  }
}

// Проверяет, что строка содержит подстроку.
inline void ExpectContains(const std::string& haystack,
                           const std::string& needle, const char* haystack_expr,
                           const char* needle_expr) {
  if (haystack.find(needle) == std::string::npos) {
    std::ostringstream stream;
    stream << "expected " << haystack_expr << " to contain " << needle_expr;
    throw Failure{stream.str()};
  }
}

// Регистрирует тест во время инициализации модуля.
class Registrar {
 public:
  // Регистрирует тест во время инициализации модуля.
  Registrar(const char* name, std::function<void()> function) {
    RegisterTest(name, std::move(function));
  }
};

}  // namespace test

#define TEST(name)                                        \
  static void name();                                     \
  static ::test::Registrar registrar_##name(#name, name); \
  static void name()

#define EXPECT_EQ(left, right) \
  ::test::ExpectEqual((left), (right), #left, #right)
#define EXPECT_TRUE(expression) \
  ::test::ExpectTrue(static_cast<bool>(expression), #expression)
#define EXPECT_FALSE(expression) \
  ::test::ExpectFalse(static_cast<bool>(expression), #expression)
#define EXPECT_CONTAINS(haystack, needle) \
  ::test::ExpectContains((haystack), (needle), #haystack, #needle)
