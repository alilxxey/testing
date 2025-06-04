/**
 * @file   MarkerTracker.cpp
 */
#include "MarkerTracker.hpp"

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/eigen.hpp>
#include <spdlog/spdlog.h>

namespace qrslam {

// ---------------------------------------------------------------------
// ctor
// ---------------------------------------------------------------------
MarkerTracker::MarkerTracker(const CameraIntrinsics& K) : K_{K} {}

// ---------------------------------------------------------------------
// public
// ---------------------------------------------------------------------
void MarkerTracker::addDetections(const std::vector<QrDetection>& dets,
                                  const Eigen::Matrix4d& T_cw,
                                  double marker_size) {
    if (dets.empty()) return;

    // Camera pose world<-camera
    Eigen::Matrix4d T_wc = T_cw.inverse();
    Eigen::Matrix3d R_wc = T_wc.block<3,3>(0,0);
    Eigen::Vector3d t_wc = T_wc.block<3,1>(0,3);

    // intrinsics
    cv::Mat Kcv = (cv::Mat_<double>(3,3) <<
                   K_.fx, 0,     K_.cx,
                   0,     K_.fy, K_.cy,
                   0,     0,     1);

    const double s = marker_size;
    std::vector<cv::Point3f> obj{
        {-s/2,-s/2,0}, { s/2,-s/2,0},
        { s/2, s/2,0}, {-s/2, s/2,0}
    };

    for (const auto& d : dets) {
        std::vector<cv::Point2f> img_pts(d.corners_px.begin(),
                                         d.corners_px.end());
        cv::Mat rvec, tvec;
        bool ok = cv::solvePnP(obj, img_pts, Kcv, cv::Mat(),
                               rvec, tvec, false,
                               cv::SOLVEPNP_ITERATIVE);
        if (!ok) {
            spdlog::warn("[MarkerTracker] solvePnP failed for {}", d.id);
            continue;
        }

        cv::Mat Rcv;
        cv::Rodrigues(rvec, Rcv);
        Eigen::Matrix3d R_cm;
        Eigen::Vector3d t_cm;
        cv::cv2eigen(Rcv, R_cm);
        cv::cv2eigen(tvec, t_cm);

        Eigen::Matrix3d R_wm = R_wc * R_cm;
        Eigen::Vector3d t_wm = R_wc * t_cm + t_wc;

        map_[d.id] = MarkerInfo{d.id, t_wm, R_wm, marker_size};
        spdlog::info("[MarkerTracker] +{}", d.id);
    }
}

void MarkerTracker::clear() { map_.clear(); }

std::optional<MarkerInfo>
MarkerTracker::get(const std::string& id) const {
    auto it = map_.find(id);
    if (it == map_.end()) return std::nullopt;
    return it->second;
}

std::vector<ProjectedMarker>
MarkerTracker::projectMarkers(const Eigen::Matrix4d& T_cw,
                              int img_w, int img_h) const {
    std::vector<ProjectedMarker> out;
    Eigen::Matrix3d R_cw = T_cw.block<3,3>(0,0);
    Eigen::Vector3d t_cw = T_cw.block<3,1>(0,3);

    for (const auto& [id, mk] : map_) {
        Eigen::Vector3d p_c = R_cw * mk.t_w + t_cw;
        if (p_c.z() <= 0.05) continue;

        double u = (K_.fx * p_c.x()) / p_c.z() + K_.cx;
        double v = (K_.fy * p_c.y()) / p_c.z() + K_.cy;
        bool inside = (u >= 0 && u < img_w && v >= 0 && v < img_h);

        out.push_back({id, cv::Point2f(float(u),float(v)),
                       inside, p_c.norm()});
    }
    return out;
}

void MarkerTracker::drawOverlay(cv::Mat& frame_bgr,
                                const Eigen::Matrix4d& T_cw) const {
    auto vis = projectMarkers(T_cw, frame_bgr.cols, frame_bgr.rows);
    for (const auto& pm : vis) {
        cv::Scalar col = pm.in_view ? cv::Scalar(0,255,0)
                                    : cv::Scalar(120,120,120);
        cv::circle(frame_bgr, pm.center_px, 6, col, 2, cv::LINE_AA);
        if (pm.in_view) {
            cv::putText(frame_bgr, pm.id,
                        pm.center_px + cv::Point2f(8,-8),
                        cv::FONT_HERSHEY_SIMPLEX, .55,
                        cv::Scalar(255,0,0), 2, cv::LINE_AA);
        }
    }
}

} // namespace qrslam
