//
// Created by inkfin on 23-1-9.
//

#include "RSM.h"
#include <iostream>

int main() {
    using namespace std;
    using namespace Eigen;

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


    Eigen::Vector2f x_t = {0.5, 1};
    Eigen::Vector2f y_t = {-0.5, -1};
    RSM rsm(x_t, y_t);

    Eigen::MatrixX4f step_df;
    rsm.run(step_df);
    cout << "step_df =\n" << step_df << endl;


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
