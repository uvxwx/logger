#pragma once

#include <utility>
#include <variant>

namespace logger {

// Минимальный контейнер для значения или ошибки.
template <typename T, typename E>
class Expected {
 public:
  // Создает успешный результат со значением.
  Expected(T value) : storage_(std::in_place_index<0>, std::move(value)) {}
  // Создает неуспешный результат с ошибкой.
  Expected(E error) : storage_(std::in_place_index<1>, std::move(error)) {}

  // Проверяет, хранится ли значение.
  bool HasValue() const noexcept { return std::holds_alternative<T>(storage_); }
  // Позволяет проверять результат как bool.
  explicit operator bool() const noexcept { return HasValue(); }

  // Возвращает значение по ссылке.
  T& Value() & { return std::get<T>(storage_); }
  // Возвращает константное значение по ссылке.
  const T& Value() const& { return std::get<T>(storage_); }
  // Возвращает значение с переносом.
  T&& Value() && { return std::get<T>(std::move(storage_)); }

  // Возвращает ошибку по ссылке.
  E& Error() & { return std::get<E>(storage_); }
  // Возвращает константную ошибку по ссылке.
  const E& Error() const& { return std::get<E>(storage_); }

  E&& Error() && { return std::get<E>(std::move(storage_)); }

 private:
  std::variant<T, E> storage_;
};

// Специализация для операций без полезного результата.
template <typename E>
class Expected<void, E> {
 public:
  // Создает успешный результат без значения.
  Expected() : storage_(std::in_place_index<0>) {}
  // Создает неуспешный результат с ошибкой.
  Expected(E error) : storage_(std::in_place_index<1>, std::move(error)) {}

  // Проверяет успешность результата.
  bool HasValue() const noexcept {
    return std::holds_alternative<std::monostate>(storage_);
  }
  // Позволяет проверять результат как bool.
  explicit operator bool() const noexcept { return HasValue(); }

  // Проверяет наличие успешного состояния.
  void Value() const { std::get<std::monostate>(storage_); }

  // Возвращает ошибку по ссылке.
  E& Error() & { return std::get<E>(storage_); }
  // Возвращает константную ошибку по ссылке.
  const E& Error() const& { return std::get<E>(storage_); }

  E&& Error() && { return std::get<E>(std::move(storage_)); }

 private:
  std::variant<std::monostate, E> storage_;
};

}  // namespace logger
