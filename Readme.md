
# QR-SLAM Demo

<img src="doc/preview.gif" align="right" width="250">

**QR-SLAM** — это демонстрационный проект, совмещающий
*visual SLAM* (на базе **[Stella VSLAM]**) и
отслеживание **QR-кодов** в реальном времени.  
Даже когда код выходит из кадра, траектория камеры
продолжает восстанавливаться, а позиция маркера
предсказывается за счёт SLAM-одометрии.

---

## 📋 Состав репозитория

```

├── cmake/                # Find-модули & helpers
│   ├── FindStellaVSLAM.cmake
│   └── version.cmake
├── scripts/              # build & run helpers
│   ├── build.sh
│   └── run\_demo.sh
├── src/                  # C++-код приложения
│   ├── App.hpp|cpp
│   ├── SlamWrapper.hpp|cpp
│   ├── MarkerTracker.hpp|cpp
│   └── utils/
└── CMakeLists.txt

````

---

## ⚙️ Сборка (Linux / macOS / Windows)

```bash
# 1. зависимости (Ubuntu 22/25 пример)
sudo apt install build-essential cmake ninja-build git \
       libopencv-dev libeigen3-dev libsuitesparse-dev \
       libyaml-cpp-dev libgflags-dev libgoogle-glog-dev sqlite3 \
       libglew-dev       # ← для Pangolin-viewer (опц.)

# 2. stella_vslam + g2o + FBoW  →  /usr/local
#    (см. docs/build_stella_vslam.md)

# 3. сам проект
./scripts/build.sh             # Release & Ninja
# или  ./scripts/build.sh -t Debug -j8
````

Файл `compile_commands.json` генерируется автоматически — пользуйтесь
`clang-tidy`, `clangd`, `include-what-you-use`.

---

## 🚀 Запуск

```bash
./scripts/run_demo.sh \
    -c share/qr_slam_demo/config.yaml \
    -v share/qr_slam_demo/orb_vocab.fbow \
    -d 0        # ID камеры
```

Клавиши в окне:

| Key           | Действие                                |
| ------------- | --------------------------------------- |
| **Space / S** | ручной рескан кадра на наличие новых QR |
| **R**         | полный сброс карты SLAM & маркеров      |
| **ESC**       | выход                                   |

---

## 🏗 Архитектура кода

| Модуль              | Ответственность                                                    |
| ------------------- | ------------------------------------------------------------------ |
| **`SlamWrapper`**   | инкапсулирует Stella VSLAM (инициализация, кадры, viewer, map I/O) |
| **`MarkerTracker`** | хранит мировые позы QR-кодов; решает PnP; проецирует в пиксели     |
| **`App`**           | UI-обвязка: камера → SLAM → Overlay + Hotkeys                      |
| **`utils/`**        | ‐ таймеры, конверсии Eigen ←→ OpenCV, математика                   |

---

## 🛨 Портирование на Android / iOS

* Соберите `stella_vslam`, `g2o`, `FBoW`, `Eigen` в **static**-режиме через
  Android NDK toolchain.
* Отключите `USE_PANGOLIN_VIEWER` (GUI не нужен).
* Используйте **Camera2 API** → `AImageReader` → cv::Mat BGR/RGB.
* Весь рендер поверх камеры делайте на стороне Java/Kotlin (или Flutter).

---

## 📄 Лицензия

Код демо распространяется под **MIT**.
Stella VSLAM и g2o — **BSD/Apache**, OpenCV — **Apache 2.0**.
Проверьте совместимость, если интегрируете в закрытый продукт.

---

> «Шуршите камеру, вращайте маркеры — и пусть SLAM никогда не теряется.»

