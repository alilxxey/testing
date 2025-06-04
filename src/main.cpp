/**
 * @file   main.cpp
 * @brief  Точка входа в приложение QR-SLAM Demo.
 *
 *  Парсит аргументы командной строки, загружает конфигурацию,
 *  и запускает основной цикл работы (SLAM + трекинг QR-кодов).
 *
 *  © 2025 YourCompany — MIT License
 */

#include <iostream>
#include <string>
#include <stdexcept>

#include <yaml-cpp/yaml.h>
#include <opencv2/opencv.hpp>

#include "App.hpp"          // основной класс-оболочка приложения
#include "SlamWrapper.hpp"  // обёртка над Stella VSLAM
#include "MarkerTracker.hpp"// трекинг и проекция QR-маркеров

static void printUsage(const char* prog) {
    std::cerr << "Usage: " << prog
              << " --config <path/to/app.yaml>"
              << " --camera <path/to/camera.yaml>"
              << " --vocab <path/to/orb_vocab.fbow>"
              << " --cam <camera_id>\n\n"
              << "  --config   Файл конфигурации приложения (app.yaml)\n"
              << "  --camera   Файл калибровки камеры (camera.yaml)\n"
              << "  --vocab    Путь к ORB-словарию (orb_vocab.fbow) для SLAM\n"
              << "  --cam      ID видеокамеры (0,1,2,...)\n";
}

int main(int argc, char** argv) {
    if (argc < 9) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    std::string app_config_path;
    std::string cam_config_path;
    std::string orb_vocab_path;
    int cam_id = 0;

    // парсим аргументы командной строки вручную
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--config" || arg == "-c") && i + 1 < argc) {
            app_config_path = argv[++i];
        }
        else if ((arg == "--camera" || arg == "-k") && i + 1 < argc) {
            cam_config_path = argv[++i];
        }
        else if ((arg == "--vocab" || arg == "-v") && i + 1 < argc) {
            orb_vocab_path = argv[++i];
        }
        else if ((arg == "--cam" || arg == "-d") && i + 1 < argc) {
            cam_id = std::stoi(argv[++i]);
        }
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    try {
        // ----------------------------------------
        // 1) Читаем конфигурацию приложения (app.yaml)
        // ----------------------------------------
        YAML::Node app_cfg = YAML::LoadFile(app_config_path);
        const auto window_cfg = app_cfg["window"];
        const int win_width  = window_cfg["width"].as<int>();
        const int win_height = window_cfg["height"].as<int>();
        const int fps_limit  = app_cfg["fps_target"].as<int>();

        const auto qr_cfg = app_cfg["qr_scan"];
        const bool qr_enable       = qr_cfg["enable"].as<bool>();
        const int qr_interval_f    = qr_cfg["interval_frame"].as<int>();
        const double marker_size_m = qr_cfg["marker_size_m"].as<double>();
        const std::string qr_detector = qr_cfg["detector"].as<std::string>();

        // ----------------------------------------
        // 2) Читаем профайл камеры (camera.yaml)
        // ----------------------------------------
        YAML::Node cam_cfg = YAML::LoadFile(cam_config_path);
        const double fx = cam_cfg["Camera"]["fx"].as<double>();
        const double fy = cam_cfg["Camera"]["fy"].as<double>();
        const double cx = cam_cfg["Camera"]["cx"].as<double>();
        const double cy = cam_cfg["Camera"]["cy"].as<double>();
        const int cam_cols = cam_cfg["Camera"]["cols"].as<int>();
        const int cam_rows = cam_cfg["Camera"]["rows"].as<int>();

        // ----------------------------------------
        // 3) Инициализируем SLAM (Stella VSLAM)
        // ----------------------------------------
        // SlamWrapper читает собственный YAML (возможно, camera.yaml или отдельный SLAM-конфиг),
        // и инициализирует движок Stella VSLAM с передачей ему orb_vocab_path.
        SlamWrapper slam(orb_vocab_path, cam_config_path, /* is_monocular = */ true);

        // ----------------------------------------
        // 4) Настраиваем трекер маркеров (проектирует QR в кадр)
        // ----------------------------------------
        qrslam::MarkerTracker::CameraIntrinsics Ki { fx, fy, cx, cy };
        qrslam::MarkerTracker marker_trk(Ki);

        // ----------------------------------------
        // 5) Настраиваем сканер QR-кодов
        // ----------------------------------------
        std::unique_ptr<QRScanner> qr_scanner;
        if (qr_detector == "zbar") {
            qr_scanner = std::make_unique<ZBarScanner>();
        } else {
            qr_scanner = std::make_unique<OpenCVScanner>();
        }

        // ----------------------------------------
        // 6) Открываем видеокамеру
        // ----------------------------------------
        cv::VideoCapture cap(cam_id);
        if (!cap.isOpened()) {
            throw std::runtime_error("Не удалось открыть камеру ID=" + std::to_string(cam_id));
        }
        cap.set(cv::CAP_PROP_FRAME_WIDTH,  cam_cols);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, cam_rows);

        const std::string window_name = "QR-SLAM Demo (ESC - exit, SPACE - scan, R - reset)";
        cv::namedWindow(window_name, cv::WINDOW_NORMAL);
        cv::resizeWindow(window_name, win_width, win_height);
        std::cout << " ESC – выход | SPACE/S – ручной скан | R – сброс SLAM+маркеров\n";

        // ----------------------------------------
        // 7) Основной цикл: захват, SLAM, QR-скан, отрисовка
        // ----------------------------------------
        double prev_time = (double)cv::getTickCount() / cv::getTickFrequency();
        double fps = 0.0;
        const double time_per_frame = (fps_limit > 0 ? 1.0 / fps_limit : 0.0);
        int frame_idx = 0;

        while (true) {
            cv::Mat frame_bgr;
            if (!cap.read(frame_bgr)) {
                std::cerr << "[WARN] не удалось захватить кадр – выходим\n";
                break;
            }

            // преобразуем BGR → серый для детекторов (SLAM внутри тоже может делать конверсию)
            cv::Mat frame_gray;
            cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);

            // -------- 7.1) Передаём кадр в SLAM и получаем T_cw (pose камеры)
            std::optional<Eigen::Matrix4d> pose_opt = slam.feedFrame(frame_bgr);
            bool slam_active = pose_opt.has_value();

            // -------- 7.2) Автоскан QR каждые qr_interval_f кадров
            if (qr_enable && qr_interval_f > 0 && (frame_idx % qr_interval_f == 0)) {
                auto detections = qr_scanner->scan(frame_bgr);
                if (!detections.empty() && slam_active) {
                    auto [new_cnt, upd_cnt] =
                        marker_trk.addDetections(detections, *pose_opt, marker_size_m);
                    if (new_cnt || upd_cnt) {
                        std::cout << "[auto-scan] +" << new_cnt
                                  << " new, " << upd_cnt << " updated\n";
                    }
                }
            }

            // -------- 7.3) Оверлей: если SLAM активен – отрисовываем маркеры
            if (slam_active) {
                marker_trk.drawOverlay(frame_bgr, *pose_opt);
                const std::string status = "TRACKING • " +
                    std::to_string(marker_trk.size()) + " code(s)";
                cv::putText(frame_bgr, status, {10, 40},
                            cv::FONT_HERSHEY_SIMPLEX, 0.6,
                            cv::Scalar(40, 220, 40), 2, cv::LINE_AA);
            } else {
                cv::putText(frame_bgr, "PRESS SPACE TO SCAN", {10, 40},
                            cv::FONT_HERSHEY_SIMPLEX, 0.6,
                            cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
            }

            // -------- 7.4) FPS-оверлей
            double now = (double)cv::getTickCount() / cv::getTickFrequency();
            double dt = now - prev_time;
            prev_time = now;
            double inst_fps = (dt > 1e-6 ? 1.0 / dt : fps);
            fps = (fps < 1e-3 ? inst_fps : 0.9 * fps + 0.1 * inst_fps);

            cv::putText(frame_bgr,
                        "FPS: " + std::to_string((int)fps),
                        {10, 20}, cv::FONT_HERSHEY_SIMPLEX,
                        0.6, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

            cv::imshow(window_name, frame_bgr);

            // -------- 7.5) Обрабатываем нажатия клавиш
            int key = cv::waitKey(1) & 0xFF;
            if (key == 27) {           // ESC
                break;
            }
            else if (key == ' ' || key == 's' || key == 'S') {  // SPACE/S
                auto detections = qr_scanner->scan(frame_bgr);
                if (!detections.empty() && slam_active) {
                    auto [new_cnt, upd_cnt] =
                        marker_trk.addDetections(detections, *pose_opt, marker_size_m);
                    std::cout << "[manual scan] +" << new_cnt
                              << " new, " << upd_cnt << " updated\n";
                } else {
                    std::cout << "[scan] QR-коды не найдены или SLAM неактивен\n";
                }
            }
            else if (key == 'r' || key == 'R') {  // R – сбросить всё
                slam.reset();
                marker_trk.clear();
                std::cout << "[INFO] SLAM и карта маркеров сброшены\n";
            }

            // -------- 7.6) Поддержка лимита FPS
            double after = (double)cv::getTickCount() / cv::getTickFrequency();
            double sleep_time = time_per_frame - (after - now);
            if (sleep_time > 0) {
                cv::waitKey((int)(sleep_time * 1000));  // ждем недостающую часть кадра
            }

            ++frame_idx;
        }

        // По выходу корректно освобождаем ресурсы
        cap.release();
        cv::destroyAllWindows();
    }
    catch (const std::exception& ex) {
        std::cerr << "Ошибка: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
