/**
 * @file   SlamWrapper.cpp
 */
#include "SlamWrapper.hpp"

#include <openvslam/system.h>
#include <openvslam/config.h>
#include <openvslam/type.h>

#ifdef USE_PANGOLIN
#   include <pangolin_viewer/viewer.h>
#endif

#include <opencv2/imgproc.hpp>
#include <spdlog/spdlog.h>

namespace qrslam {

//-------------------------------------------------------------
// ctor / dtor
//-------------------------------------------------------------
SlamWrapper::SlamWrapper(std::string cfg_path,
                         std::string vocab_path,
                         bool use_viewer)
    : use_viewer_{use_viewer}
{
    // загружаем YAML-конфигурацию камеры
    cfg_ = std::make_shared<openvslam::config>(cfg_path);

    // инициализируем SLAM-движок
    sys_ = std::make_unique<openvslam::system>(cfg_, vocab_path);
}

SlamWrapper::~SlamWrapper() {
    stop();
}

//-------------------------------------------------------------
// start / stop
//-------------------------------------------------------------
void SlamWrapper::start() {
    if (!sys_) return;

    sys_->startup();

#ifdef USE_PANGOLIN
    if (use_viewer_) {
        viewer_ = std::make_unique<pangolin_viewer::viewer>(
            openvslam::util::yaml_optional_ref(cfg_->node_, "PangolinViewer"));
        viewer_->launch_3dviewer();
        sys_->set_viewer(viewer_.get());
    }
#endif
}

void SlamWrapper::stop() {
    if (sys_) {
        sys_->shutdown();   // корректное завершение потоков SLAM
    }
#ifdef USE_PANGOLIN
    if (viewer_) {
        viewer_->request_terminate();
        viewer_.reset();
    }
#endif
}

//-------------------------------------------------------------
// кадры
//-------------------------------------------------------------
std::optional<Eigen::Matrix4d>
SlamWrapper::feedFrame(const cv::Mat& frame_rgb, double timestamp) {
    if (!sys_) return std::nullopt;

    // openvslam ожидает RGB-изображение (8UC3)
    if (frame_rgb.empty() || frame_rgb.type() != CV_8UC3) {
        spdlog::warn("SlamWrapper::feedFrame: wrong frame format");
        return std::nullopt;
    }

    auto T_cw = sys_->feed_monocular_frame(frame_rgb, timestamp);
    if (T_cw.isIdentity()) return std::nullopt;
    return T_cw;
}

std::optional<Eigen::Matrix4d> SlamWrapper::getCurrentPose() const {
    if (!sys_) return std::nullopt;
    const auto& map_db = sys_->get_map_database();
    auto cam = map_db->get_current_cam_pose();
    if (cam.isIdentity()) return std::nullopt;
    return cam;
}

//-------------------------------------------------------------
// reset / map I/O
//-------------------------------------------------------------
void SlamWrapper::reset() {
    if (sys_) {
        sys_->reset();
    }
    spdlog::info("SLAM reset");
}

bool SlamWrapper::saveMap(const std::string& path) const {
    if (!sys_) return false;
    return sys_->save_map_database(path);
}

bool SlamWrapper::loadMap(const std::string& path) {
    if (!sys_) return false;
    return sys_->load_map_database(path);
}

} // namespace qrslam
