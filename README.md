### va
Поддерживаемые операционные системы: Linux

### va-record

Функционал:

- Захват видео потоков с ip камер по протоколу rtsp и сохранение в файлы заданной длительностью.
  Поддерживаемые форматы H264, H265

Параметры запуска:
- путь до файла источников видео потоков в формате json, по умолчанию sources.json
```bash
--source-file
```
- префикс пути для хранения видео архива, по умолчанию /tmp/va
```bash
--prefix-archive-path
```
- длительность видео сегмента в секундах, по умолчанию 10
```bash
--duration-file
```
- уровень логирования приложения (debug, info, warning, error), по умолчанию info
```bash
--log-level
```

Пример файла со списком видео источников:
```json
[
  {
    "id":"camera-1",
    "url":"192.168.0.1:554/stream",
    "username": "admin",
    "password": "admin"
  },
  {
    "id":"camera-2",
    "url":"192.168.0.2:554/stream",
    "username": "admin",
    "password": "admin"
  }
]
```

### va-hls

Функционал:

- Реализация HLS сервера для архивного видео. Запрос видео делается за конкретный час архива
- Реализация HLS сервера для live видео

Параметры запуска:
- префикс пути для хранения видео архива, по умолчанию /tmp/va
```bash
--prefix-archive-path
```
- порт сервера, по умолчанию 8888
```bash
--port
```
- уровень логирования приложения (debug, info, warning, error), по умолчанию info
```bash
--log-level
```

Пример запроса HLS с помощью ffplay, время в UTC
```bash
ffplay http://localhost:8888/camera-1/2024/04/17/05/index.m3u8
```
Пример запроса HLS с помощью ffplay (live)
```bash
ffplay http://localhost:8888/camera-1/index.m3u8
```
Задержка Live трансляции от 20 - 40  секунд при файлах (5 с)

Задержка Live трансляции от 40 - 60  секунд при файлах (10 с)

### va-inference

Функционал:
- Отправка данных на удаленный инференс, получение результата и сохранение в файл рядом с видео файлом

Параметры запуска:
- префикс пути для хранения видео архива, по умолчанию /tmp/va
```bash
--prefix-archive-path
```
- адрес инференс сервиса, по умолчанию localhost
```bash
--inference-server-address 
```
- порт инференс сервиса, по умолчанию 3030
```bash
--inference-server-port 
```
- количество паралелльных потоков инференса, по умолчанию 1
```bash
--num-threads
```
- уровень логирования приложения (debug, info, warning, error), по умолчанию info
```bash
--log-level
```

В репозитории на данный момент присутствиет пример для инференса на YOLO V5

### Сборка

Зависимости:
- boost версия 1.84 и выше
- ffmpeg версия 6.0 и выше

Компилятор:
- Версия компилятора gcc-13.1.0 (C++20) и выше