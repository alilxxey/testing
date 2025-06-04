#pragma once
/**
 * @file   MarkerTracker.hpp
 * @brief  Управляет картой QR-маркеров в мировой СК и их проекциями.
 *
 * © 2025 YourCompany — MIT License.
 */
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <array>

#include <Eigen/Core>
#include <opencv2/core.hpp>

namespace qrslam {

    // ---------- входные/выходные структуры ---------------------------------
    struct QrDetection {
        std::string               id;                ///< строка из QR-кода
        std::array<cv::Point2f,4> corners_px;        ///< углы (пиксели)
    };

    struct MarkerInfo {
        std::string     id;
        Eigen::Vector3d t_w;       ///< центр маркера в мировой СК
        Eigen::Matrix3d R_w;       ///< ориентация
        double          size;      ///< сторона квадрата, м
    };

    struct ProjectedMarker {
        std::string id;
        cv::Point2f center_px;
        bool        in_view;
        double      depth_m;
    };

    // ---------- класс-обёртка ----------------------------------------------
    class MarkerTracker {
    public:
        struct CameraIntrinsics { double fx, fy, cx, cy; };

        explicit MarkerTracker(const CameraIntrinsics& K);

        /** Добавить/обновить по новым детекциям. */
        void addDetections(const std::vector<QrDetection>& dets,
                           const Eigen::Matrix4d& T_cw,
                           double marker_size_m);

        void clear();
        std::size_t size() const { return map_.size(); }

        std::optional<MarkerInfo> get(const std::string& id) const;

        /** Вернуть спроектированные центры всех маркеров. */
        std::vector<ProjectedMarker>
        projectMarkers(const Eigen::Matrix4d& T_cw,
                       int img_w, int img_h) const;

        /** Нарисовать кружок + подпись ID на кадре. */
        void drawOverlay(cv::Mat& frame_bgr,
                         const Eigen::Matrix4d& T_cw) const;

    private:
        CameraIntrinsics                              K_;
        std::unordered_map<std::string, MarkerInfo>   map_;
    };

} // namespace qrslam
