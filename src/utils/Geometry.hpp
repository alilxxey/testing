#pragma once
/**
 * @file   Geometry.hpp
 * @brief  Набор маленьких in-place-inline-вспомогашек для конверсий
 *         и базовых операций с позами / точками (Eigen ⇄ OpenCV).
 *
 *         Всё в header-only стиле, без зависимостей кроме
 *         <Eigen/Core> и <opencv2/core.hpp>.
 *
 * © 2025 YourCompany — MIT License.
 */

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <opencv2/core.hpp>

namespace qrslam::geom {

//--------------------------------------------------------------
// Типы
//--------------------------------------------------------------
using Mat44d = Eigen::Matrix4d;
using Vec3d  = Eigen::Vector3d;
using Mat33d = Eigen::Matrix3d;

/**
 * @brief  Собрать однородную 4×4 матрицу из R|t
 *         (R — 3×3,  t — 3×1).
 */
inline Mat44d Rt2T(const Mat33d& R, const Vec3d& t) {
    Mat44d T = Mat44d::Identity();
    T.block<3,3>(0,0) = R;
    T.block<3,1>(0,3) = t;
    return T;
}

/**
 * @brief  Разложить 4×4 на (R,t).
 */
inline void T2Rt(const Mat44d& T, Mat33d& R, Vec3d& t) {
    R = T.block<3,3>(0,0);
    t = T.block<3,1>(0,3);
}

/**
 * @brief  Инверсия SE(3):  T⁻¹ = [ Rᵀ | -Rᵀ·t ].
 */
inline Mat44d invertSE3(const Mat44d& T) {
    Mat33d R = T.block<3,3>(0,0);
    Vec3d  t = T.block<3,1>(0,3);
    Mat44d Ti = Mat44d::Identity();
    Ti.block<3,3>(0,0) = R.transpose();
    Ti.block<3,1>(0,3) = -R.transpose() * t;
    return Ti;
}

/**
 * @brief  Умножение SE(3) * точка (3-D ⇒ 3-D).
 */
inline Vec3d transformPoint(const Mat44d& T, const Vec3d& p) {
    return T.block<3,3>(0,0) * p + T.block<3,1>(0,3);
}

/**
 * @brief  Проекция 3-D точки в пиксели камеры K·[R|t]·P.
 * @param  K   3×3 (fx,0,cx; 0,fy,cy; 0,0,1)
 * @return (u,v,depth)
 */
inline Eigen::Vector3d projectPoint(const Mat44d& T_cw,
                                    const Eigen::Matrix3d& K,
                                    const Vec3d& P_w) {
    Vec3d p_c = transformPoint(T_cw, P_w);  // в камеру
    double z  = p_c.z() + 1e-12;
    double u  = (K(0,0)*p_c.x() + K(0,2)*z) / z;
    double v  = (K(1,1)*p_c.y() + K(1,2)*z) / z;
    return {u, v, z};
}

/**
 * @brief  cv::Mat (3×3/3×1) → Eigen (in-place, без копии данных).
 *         Используйте осторожно: структура памяти должна совпадать.
 */
template<int R, int C>
inline Eigen::Map<Eigen::Matrix<double,R,C,Eigen::RowMajor>>
cv2eigenRef(cv::Mat& m) {
    static_assert(R == Eigen::Dynamic || C == Eigen::Dynamic,
                  "Specify correct dimensions");
    return Eigen::Map<Eigen::Matrix<double,R,C,Eigen::RowMajor>>(
        m.ptr<double>(), m.rows, m.cols);
}

/**
 * @brief  Eigen → cv::Mat (copy).
 */
template<int R, int C>
inline cv::Mat eigen2cv(const Eigen::Matrix<double,R,C>& M) {
    cv::Mat out(int(R), int(C), CV_64F);
    std::memcpy(out.ptr<double>(), M.data(), sizeof(double)*R*C);
    return out;
}

/**
 * @brief  Rodrigues:  cv::Mat rvec (3×1) ⇄ Eigen::Matrix3d.
 */
inline Mat33d rodriguesToMat(const cv::Mat& rvec) {
    cv::Mat Rcv;
    cv::Rodrigues(rvec, Rcv);
    Mat33d R;
    cv::cv2eigen(Rcv, R);
    return R;
}
inline cv::Mat matToRodrigues(const Mat33d& R) {
    cv::Mat Rcv, rvec;
    cv::eigen2cv(R, Rcv);
    cv::Rodrigues(Rcv, rvec);
    return rvec; // CV_64F 3×1
}

} // namespace qrslam::geom
