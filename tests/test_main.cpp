#include <exception>
#include <iostream>

#include "test_framework.hpp"

// Запускает все зарегистрированные тесты и печатает итог.
int main() {
  int failed = 0;

  for (const auto& test_case : test::Registry()) {
    try {
      test_case.function();
      std::cout << "[PASS] " << test_case.name << '\n';
    } catch (const std::exception& error) {
      ++failed;
      std::cerr << "[FAIL] " << test_case.name << ": " << error.what() << '\n';
    } catch (...) {
      ++failed;
      std::cerr << "[FAIL] " << test_case.name << ": unknown exception\n";
    }
  }

  if (failed != 0) {
    std::cerr << failed << " test(s) failed\n";
    return 1;
  }

  std::cout << test::Registry().size() << " test(s) passed\n";
  return 0;
}
