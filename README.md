# Logger

Этот репозиторий реализует все три части тестового задания. Тяжело было однозначно интерпретировать ограничение про исключения, поэтому исключения не используются совсем.
Docker-образ проекта можно найти справа.

## Сборка

```bash
make build
make test
```

## Цели CMake

Библиотеки:

* `logger_static`
* `logger_shared`

Программы:

* `logger_demo`
* `udp_logger_demo`
* `statistics_receiver`

## Библиотека логирования

Основные публичные типы:

* `logger::Expected<T, E>`
* `logger::Sink`
* `logger::FileSink`
* `logger::UdpSink`
* `logger::Logger`
* `logger::Statistics`

Поддерживаемые уровни логирования:

* `debug`
* `info`
* `error`

Разбор уровня выполняется без учета регистра. Неизвестные значения
отклоняются.

`Logger` отфильтровывает сообщения ниже текущего минимального уровня.
Минимальный уровень можно менять во время работы через `SetMinLevel()`.

Перегрузка `Logger::Log(message)` использует текущий минимальный уровень как
уровень сообщения. Пустые сообщения отклоняются на уровне `Logger`.

## Формат строк в файле

Каждая запись пишется в одну строку:

```text
2026-07-16T14:32:51.482Z [INFO] Program started
```

В тексте сообщения экранируются:

* `\n`
* `\r`
* `\t`
* `\\`

Файловый sink открывает файл один раз в режиме append и делает `flush` после
каждой записи.

## Формат UDP-пакета

UDP-датаграмма сериализуется вручную:

```text
1 byte    уровень логирования
8 bytes   Unix timestamp в миллисекундах
4 bytes   длина сообщения
N bytes   полезная нагрузка сообщения
```

Все целочисленные поля передаются в сетевом порядке байт. Максимальный размер
сообщения ограничен 60 KiB.

## Консольные программы

Логирование в файл:

```bash
./build/logger_demo <log-file> <minimum-level>
```

Логирование по UDP:

```bash
./build/udp_logger_demo <host> <port> <minimum-level>
```

Поддерживаемый ввод:

```text
debug message text
info message text
error message text
message without a recognized level
```

Служебные команды:

* `:help`
* `:level debug`
* `:level info`
* `:level error`
* `:quit`

## Статистики

Запуск:

```bash
./build/statistics_receiver <bind-address> <port> <N> <T-seconds>
```

Пример:

```bash
./build/statistics_receiver 0.0.0.0 9000 10 5
```
## Тесты

Запуск:

```bash
make test
```

Тесты покрывают:

* фильтрацию по уровню и распространение ошибок sink
* форматирование и append-поведение файлового sink
* синхронизацию блокирующей очереди
* порядок обработки команд и `:level` в программе
* сериализацию и валидацию UDP-пакетов
* накопление статистики
* отправку UDP-сообщений на loopback

## Форматирование и статический анализ

Форматирование:

```bash
make format
```

Статический анализ:

```bash
make tidy
```

## Воспроизводимое окружение в контейнере

Базовый образ в `Dockerfile` зафиксирован по хешу. Получаемый образ собирает проект и запускает тесты.

Сборка Docker:

```bash
make container-build
```

Сборка Podman:

```bash
make container-build CONTAINER_TOOL=podman
```
