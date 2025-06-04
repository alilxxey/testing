#pragma once
/**
 * @file   SlamWrapper.hpp
 * @brief  Упрощённый фасад к openvslam::system.
 *
 * © 2025 YourCompany. MIT License.
 */
#include <string>
#include <memory>
#include <optional>

#include <Eigen/Core>
#include <opencv2/core.hpp>

namespace openvslam {
class system;
class config;
} // namespace openvslam

namespace pangolin_viewer {
class viewer;
} // namespace pangolin_viewer

namespace qrslam {

class SlamWrapper final {
public:
    /**
     * @param cfg_path    Путь к config.yaml (камера + параметры ORB).
     * @param vocab_path  Путь к словарю ORB (orb_vocab.fbow).
     * @param use_viewer  true → создаётся окно Pangolin с 3-D картой.
     */
    SlamWrapper(std::string cfg_path,
                std::string vocab_path,
                bool use_viewer = false);

    ~SlamWrapper();

    /// Начать работу (вызовет openvslam::system::startup).
    void start();

    /// Остановить работу (shutdown + join поток viewer’а).
    void stop();

    /// Передать кадр (RGB) и получить матрицу T_cw (4×4, double).
    /// Возвращает std::nullopt, если SLAM ещё не инициализирован.
    std::optional<Eigen::Matrix4d>
    feedFrame(const cv::Mat& frame_rgb, double timestamp);

    /// Текущая поза камеры (T_cw). std::nullopt, если поза неизвестна.
    std::optional<Eigen::Matrix4d> getCurrentPose() const;

    /// Сброс SLAM (очищает карту, запускает заново).
    void reset();

    /// Сохранение / загрузка карты.
    bool saveMap(const std::string& path) const;
    bool loadMap(const std::string& path);

private:
    std::shared_ptr<openvslam::config> cfg_;
    std::unique_ptr<openvslam::system> sys_;

    // Viewer (опция)
    bool use_viewer_ = false;
    std::unique_ptr<pangolin_viewer::viewer> viewer_;

    // forbid copy/move
    SlamWrapper(const SlamWrapper&)            = delete;
    SlamWrapper& operator=(const SlamWrapper&) = delete;
    SlamWrapper(SlamWrapper&&)                 = delete;
    SlamWrapper& operator=(SlamWrapper&&)      = delete;
};

} // namespace qrslam
