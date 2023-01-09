//
// Created by inkfin on 23-1-9.
//

#include "RSM.h"
#include <iostream>
#include <Eigen/Core>
#include <Eigen/Geometry>

void rotateNormal(const Eigen::Vector4f& position,
                  const Eigen::Vector4f& normal,
                  Eigen::Vector4f& new_normal,
                  const Eigen::Matrix4f& view,
                  float yaw,
                  float pitch) {
    const Eigen::Matrix4f t_inv_4f = view.inverse();
    const Eigen::Matrix3f t_inv = t_inv_4f.topLeftCorner<3, 3>();
    const Eigen::Vector3f position_3f = position.head<3>();
    const Eigen::Vector3f normal_3f = normal.head<3>();

    // center points normal vector
    Eigen::Vector3f vPosHome = t_inv * position_3f;
    vPosHome.normalize();
    // surfel normal vector
    Eigen::Vector3f vNormRad = t_inv * normal_3f;
    vNormRad.normalize();

    // get eye coordinates
    Eigen::Vector3f up = Eigen::Vector3f::UnitY();
    Eigen::Vector3f z = -vPosHome;
    Eigen::Vector3f x = up.cross(z);
    x.normalize();
    Eigen::Vector3f y = z.cross(x);
    z.normalize();

    // rotate normal
    Eigen::Matrix3f rot;
    rot = Eigen::AngleAxisf(yaw * M_PI, y)
            * Eigen::AngleAxisf(pitch * M_PI, x);
    Eigen::Vector3f new_vNormRad = rot * vNormRad;

    Eigen::Vector3f new_normal_3f = t_inv.inverse() * new_vNormRad;
    new_normal.head<3>() = new_normal_3f;
    new_normal(3) = normal(3);

    // reference https://zhuanlan.zhihu.com/p/66384929

    /*
    // get center look at matrix
    Eigen::Matrix3f look_at = Eigen::Matrix3f::Identity();
    look_at(0, Eigen::seq(0, 2)) = x.transpose();
    look_at(1, Eigen::seq(0, 2)) = y.transpose();
    look_at(2, Eigen::seq(0, 2)) = z.transpose();

    // get relative transform (normal -> look_at)
    Eigen::Vector3f relative_normal = look_at * normal_3f;

    // rotate normal
    Eigen::Matrix3f mat;
    mat = Eigen::AngleAxisf(yaw * M_PI, Eigen::Vector3f::UnitY())
            * Eigen::AngleAxisf(pitch * M_PI, Eigen::Vector3f::UnitX());
    new_normal.head<3>() = mat * relative_normal;

    // make back to original space
    new_normal.head<3>() =  * relative_normal;
    */
}

int main() {
    using namespace std;
    using namespace Eigen;

    Eigen::Vector4f position(1,1,1,1);
    Eigen::Vector4f normal(0,1,0,0);
    Eigen::Vector4f new_normal;
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    float yaw = 2.0f;
    float pitch = 0.0f;

    rotateNormal(position, normal, new_normal, view, yaw, pitch);

    cout << new_normal << endl;

    cout << Eigen::AngleAxisf(M_PI, Eigen::Vector3f::UnitX())
        * Eigen::Vector3f(0, 1, 0) << endl;

    /*
    MatrixXd A(5,2);
    A <<    1,1,
            2,1,
            3,1,
            4,1,
            7,1;
    cout << "A\n" << A << endl;
    VectorXd b(5);
    b << 8,9,13,15,25;

    cout << "b\n" << b << endl;
    VectorXd result;
//    cout << A.bdcSvd(ComputeThinU | ComputeThinV).solve(b) << endl;
    result = A.householderQr().solve(b);
    cout << result << endl;  // fast
    result = A.colPivHouseholderQr().solve(b);
    cout << result << endl;  // slow
    cout << A.rows() << " " << A.cols() << endl;
    */


//    Eigen::Vector2f x_t = {0.5, 1};
//    Eigen::Vector2f y_t = {-0.5, -1};
//    RSM rsm;
//    rsm.x_t = x_t;
//    rsm.y_t = y_t;
//
//    Eigen::MatrixX4f step_df;
//    rsm.run();
//    cout << "step_df =\n" << step_df << endl;


/*
    // test polyFit()
    MatrixX2f A_(4, 2);
    A_ <<   0.5, -0.5,
            0.5,   -1,
            1, -0.5,
            1,   -1;
    VectorXf b_(4);
    b_ << 0.867838,0.0772322,0.258828,0.0230341;
    rsm.polyFit(A_, b_, 1);

    // test predict
    MatrixX2f X_test(1,2);
    X_test << 0.5, -0.5;
    cout << "predict = " << rsm.predict(X_test) << endl;
*/

    /*
    // test appendMat()
    Eigen::MatrixX4f A1 = Eigen::MatrixXf::Random(4, 4);
    Eigen::Matrix4f A2 = Eigen::MatrixXf::Random(4,4);
    cout << "A1:\n" << A1 << endl;
    cout << "A2:\n" << A2 << endl;
    rsm.appendMat(A1, A2);
//    A1.conservativeResize(4, 8);
//    cout << "A1:\n" << A1 << endl;
//    A1(Eigen::all, Eigen::seq(4, 7)) = A2;
    cout << "A3:\n" << A1 << endl;
     */


}
