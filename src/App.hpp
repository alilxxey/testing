#pragma once
/**
 * @file   App.hpp
 * @brief  Главный класс демо-приложения: камера → SLAM → QR-маркер-трек.
 *
 * © 2025 YourCompany.  MIT License.
 */
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>

#include <Eigen/Core>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>

namespace openvslam {
class system;
class config;
} // namespace openvslam

namespace qrslam {

//-------------------------------------------------------------
//
// Структуры данных
//
struct MarkerPose {
    std::string id;             ///< Декодированная строка из QR-кода
    Eigen::Vector3d t_w;        ///< Центр маркера в мировой СК (метры)
    Eigen::Matrix3d R_w;        ///< Ориентация маркера (опц.)
};

struct AppParams {
    std::string config_path;    ///< openvslam config.yaml
    std::string vocab_path;     ///< ORB словарь .fbow
    int         cam_id   = 0;   ///< ID видеоустройства
    int         width    = 1280;
    int         height   = 720;
    double      cam_fps  = 60.0;
    double      marker_size = 0.040; ///< физический размер QR-кода (м)
};

//-------------------------------------------------------------
//
// Класс приложения
//
class App {
public:
    explicit App(const AppParams& params);
    ~App();

    /// Основной цикл (блокирующий). ESC — выход.
    void run();

private:
    // — внутренние сервисы —
    void handleHotkey(int key, const cv::Mat& frame_rgb, double timestamp);
    void detectAndRegisterMarkers(const cv::Mat& frame_gray,
                                  const cv::Mat& frame_rgb,
                                  const Eigen::Matrix4d& T_cw);           // PnP
    void drawOverlay(cv::Mat& frame_rgb,
                     const Eigen::Matrix4d& T_cw) const;                 // UI

    // — поля —
    AppParams                               p_;
    std::shared_ptr<openvslam::config>      cfg_;
    std::unique_ptr<openvslam::system>      slam_;

    cv::VideoCapture                        cap_;
    cv::QRCodeDetector                      qrdet_;

    std::unordered_map<std::string, MarkerPose> markers_;  // карта маркеров
    bool                                     need_scan_   = true;  // стартовая инициализация
};

} // namespace qrslam
