/**
* @file   main.cpp
 * @brief  Точка входа в приложение QR-SLAM Demo.
 *
 *  Этот файл больше не содержит прямых вызовов
 *  <openvslam/config.h> или <openvslam/system.h>. Всё, что касается
 *  инициализации SLAM и QR-трекинга, теперь внутри класса App.
 *
 *  Пример использования:
 *    ./qr_slam_demo \
 *      --config ../config/app.yaml \
 *      --camera ../config/camera.yaml \
 *      --vocab  ../config/orb_vocab.fbow \
 *      --cam    0
 *
 *  © 2025 YourCompany — MIT License
 */

#include <iostream>
#include <exception>

#include "App.hpp"  // здесь скрыта вся логика инициализации SLAM + QR-tracker

static void printUsage(const char* prog) {
    std::cerr << "Usage: " << prog
              << " --config <path/to/app.yaml>"
              << " --camera <path/to/camera.yaml>"
              << " --vocab <path/to/orb_vocab.fbow>"
              << " --cam <camera_id>\n\n"
              << "  --config   Файл конфигурации приложения (app.yaml)\n"
              << "  --camera   Файл калибровки камеры (camera.yaml)\n"
              << "  --vocab    Путь к ORB-словарию (orb_vocab.fbow)\n"
              << "  --cam      ID видеокамеры (0,1,2,...)\n";
}

int main(int argc, char** argv) {
    // Если мало аргументов, просто выведем справку
    if (argc < 9) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    try {
        // Класс App.hpp внутри себя сам парсит те же самые флаги:
        //   --config, --camera, --vocab, --cam
        //
        // В конструкторе App происходит вся загрузка YAML-ов,
        // инициализация SLAM, MarkerTracker, QRScanner и т.п.
        qrslam::App application(argc, argv);

        // Запускаем основной цикл
        return application.run();
    }
    catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Unknown fatal error\n";
        return EXIT_FAILURE;
    }
}
