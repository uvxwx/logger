#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "logger/sink.hpp"

namespace logger {

// Сериализует запись и отправляет ее одной UDP-датаграммой.
class UdpSink final : public Sink {
 public:
  // Создает UDP sink для заданного адреса назначения.
  static Expected<std::unique_ptr<Sink>, Error> Make(std::string host,
                                                     std::uint16_t port);

  ~UdpSink() override;

  UdpSink(const UdpSink&) = delete;
  UdpSink& operator=(const UdpSink&) = delete;
  UdpSink(UdpSink&&) = delete;
  UdpSink& operator=(UdpSink&&) = delete;

  // Отправляет одну запись по UDP.
  Expected<void, Error> Write(const LogRecord& record) noexcept override;

 private:
  // Инициализирует sink готовым сокетом и адресом.
  UdpSink(std::string host, std::uint16_t port, int socket_fd,
          std::string address_bytes);

  std::string host_;
  std::uint16_t port_ = 0;
  int socket_fd_ = -1;
  std::string address_bytes_;
};

}  // namespace logger
