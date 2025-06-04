/**
 * @file   App.cpp
 * @brief  Реализация qrslam::App
 */
#include "App.hpp"

#include <openvslam/config.h>
#include <openvslam/system.h>

#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

namespace qrslam {

//-------------------------------------------------------------
// ctor / dtor
//-------------------------------------------------------------
App::App(const AppParams& params) : p_{params} {

    // --- SLAM конфигурация и система ----------------------------------
    cfg_  = std::make_shared<openvslam::config>(p_.config_path);
    slam_ = std::make_unique<openvslam::system>(cfg_, p_.vocab_path);
    slam_->startup();

    // --- камера --------------------------------------------------------
    cap_.open(p_.cam_id, cv::CAP_ANY);
    if (!cap_.isOpened())
        throw std::runtime_error("Cannot open camera " + std::to_string(p_.cam_id));

    cap_.set(cv::CAP_PROP_FRAME_WIDTH,  p_.width);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, p_.height);
    cap_.set(cv::CAP_PROP_FPS,          p_.cam_fps);

    std::cout << "QR-SLAM demo started  (ESC exit | SPACE scan | R reset)\n";
}

App::~App() {
    if (slam_) {
        slam_->shutdown();
    }
}

//-------------------------------------------------------------
// public
//-------------------------------------------------------------
void App::run() {
    const std::string kWin = "QR-SLAM Demo";
    cv::namedWindow(kWin, cv::WINDOW_NORMAL);
    double t0 = static_cast<double>(cv::getTickCount());

    while (true) {
        cv::Mat frame_bgr;
        cap_ >> frame_bgr;
        if (frame_bgr.empty()) break;

        cv::Mat frame_rgb;
        cv::cvtColor(frame_bgr, frame_rgb, cv::COLOR_BGR2RGB);
        cv::Mat frame_gray;
        cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);

        // ------ timestamp в секундах ------
        double ts = (cv::getTickCount() - t0) / cv::getTickFrequency();

        // ------ SLAM ------
        Eigen::Matrix4d T_cw = slam_->feed_monocular_frame(frame_rgb, ts);

        // ------ первичный / ручной скан ------
        if (need_scan_) {
            detectAndRegisterMarkers(frame_gray, frame_rgb, T_cw);
        }

        // ------ overlay ------
        drawOverlay(frame_bgr, T_cw);
        cv::imshow(kWin, frame_bgr);

        // ------ hotkeys ------
        int key = cv::waitKey(1) & 0xFF;
        if (key == 27) break;             // ESC
        handleHotkey(key, frame_rgb, ts);
    }
}

//-------------------------------------------------------------
// private helpers
//-------------------------------------------------------------
void App::handleHotkey(int key, const cv::Mat& frame_rgb, double ts) {
    switch (key) {
        case ' ': case 's': {                         // manual scan
            cv::Mat frame_gray;
            cv::cvtColor(frame_rgb, frame_gray, cv::COLOR_RGB2GRAY);
            Eigen::Matrix4d T_cw = slam_->get_map_database()->get_current_cam_pose();
            detectAndRegisterMarkers(frame_gray, frame_rgb, T_cw);
            break;
        }
        case 'r': {                                   // reset
            markers_.clear();
            slam_->reset();
            need_scan_ = true;
            std::cout << "[INFO] reset\n";
            break;
        }
        default: break;
    }
}

void App::detectAndRegisterMarkers(const cv::Mat& frame_gray,
                                   const cv::Mat& frame_rgb,
                                   const Eigen::Matrix4d& T_cw) {
    std::vector<cv::Point>       pts;
    std::vector<cv::Point2f>     corners;
    std::vector<std::string>     datas;

    bool ok = qrdet_.detectAndDecodeMulti(frame_gray, datas, corners, pts);
    if (!ok || datas.empty()) {
        std::cout << "[scan] none\n";
        need_scan_ = false;
        return;
    }

    // развернём в векторы по 4 угла
    size_t num = datas.size();
    for (size_t i = 0; i < num; ++i) {
        std::string id = datas[i];
        if (id.empty()) continue;

        // corners come as Nx4, contiguous
        cv::Point2f c2d[4];
        std::copy_n(&corners[i*4], 4, c2d);

        // --- solvePnP ---
        const double s = p_.marker_size;
        std::vector<cv::Point3f> obj{
            {-s/2, -s/2, 0},  {s/2, -s/2, 0},
            {s/2,  s/2, 0},  {-s/2,  s/2, 0}
        };
        cv::Mat rvec, tvec;
        cv::Mat K = (cv::Mat_<double>(3,3) <<
                     cfg_->camera_->fx_, 0, cfg_->camera_->cx_,
                     0, cfg_->camera_->fy_, cfg_->camera_->cy_,
                     0, 0, 1);
        cv::solvePnP(obj, std::vector<cv::Point2f>(c2d, c2d+4),
                     K, cv::Mat(), rvec, tvec, false,
                     cv::SOLVEPNP_ITERATIVE);

        cv::Mat Rcv;
        cv::Rodrigues(rvec, Rcv);
        Eigen::Matrix3d R_cm;
        Eigen::Vector3d t_cm;
        cv::cv2eigen(Rcv, R_cm);
        cv::cv2eigen(tvec, t_cm);

        // --- мировая поза маркера ---
        Eigen::Matrix3d R_wc = T_cw.block<3,3>(0,0).transpose();
        Eigen::Vector3d t_wc = -R_wc * T_cw.block<3,1>(0,3);

        Eigen::Matrix3d R_wm = R_wc * R_cm;
        Eigen::Vector3d t_wm = R_wc * t_cm + t_wc;

        markers_[id] = MarkerPose{id, t_wm, R_wm};
        std::cout << "[scan] +" << id << "\n";
    }
    need_scan_ = false;
}

void App::drawOverlay(cv::Mat& frame_rgb,
                      const Eigen::Matrix4d& T_cw) const {
    if (markers_.empty()) return;

    // параметры камеры
    const auto& cam = cfg_->camera_;
    cv::Mat K = (cv::Mat_<double>(3,3) <<
                 cam->fx_, 0, cam->cx_,
                 0, cam->fy_, cam->cy_,
                 0, 0, 1);

    Eigen::Matrix3d R_wc = T_cw.block<3,3>(0,0).transpose();
    Eigen::Vector3d t_wc = -R_wc * T_cw.block<3,1>(0,3);

    for (const auto& [id, mk] : markers_) {
        // вектор от камеры к маркеру
        Eigen::Vector3d p_c = R_wc * (mk.t_w - t_wc);

        if (p_c.z() <= 0.1) continue; // за камерой ← не рисуем

        // проектируем центр
        cv::Point2d uv(
            (cam->fx_ * p_c.x()) / p_c.z() + cam->cx_,
            (cam->fy_ * p_c.y()) / p_c.z() + cam->cy_);

        cv::Scalar col = {0, 255, 0};
        cv::circle(frame_rgb, uv, 6, col, 2, cv::LINE_AA);
        cv::putText(frame_rgb, id, uv + cv::Point2d(8, -8),
                    cv::FONT_HERSHEY_SIMPLEX, .6, {255,0,0}, 2, cv::LINE_AA);
    }
}

} // namespace qrslam
