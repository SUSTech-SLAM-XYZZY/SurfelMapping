//
// Created by inkfin on 23-1-8.
//

#include "RSM.h"
#include "Error.h"

void RSM::appendMat(Eigen::MatrixX4f& mat1, const Eigen::Matrix4f& mat2) {
    mat1.conservativeResize(mat1.rows() + mat2.rows(), mat1.cols());
    mat1(Eigen::seq(mat1.rows() - mat2.rows(), mat1.rows()-1), Eigen::all) = mat2;
}

Eigen::MatrixXf RSM::pinv_eigen_based(const Eigen::MatrixXf& origin, const float er) {
    Eigen::JacobiSVD<Eigen::MatrixXf> svd_holder(origin,
                                                 Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::MatrixXf U = svd_holder.matrixU();
    const Eigen::MatrixXf& V = svd_holder.matrixV();
    Eigen::MatrixXf D = svd_holder.singularValues();

    Eigen::MatrixXf S(V.cols(), U.cols());
    S.setZero();

    for (unsigned int i = 0; i < D.size(); i++) {
        if (D(i, 0) > er) {
            S(i, i) = 1 / D(i, 0);
        } else {
            S(i, i) = 0;
        }
    }

    // pinv_matrix = V * S * U^T
    return V * S * U.transpose();
}

RSM::RSM(Eigen::Vector2f x_t, Eigen::Vector2f y_t):
    x_t(std::move(x_t)),
    y_t(std::move(y_t))
{}

void RSM::run(Eigen::MatrixX4f& step_df) {
    for (int t=0; t<max_iter; t++) {
        // output model()
        Eigen::Matrix4f tmp_df;  // 4x4
        int _i = 0;
        for (float x: x_t) {
            for (float y: y_t) {
                tmp_df(_i, COLS::X) = x;
                tmp_df(_i, COLS::Y) = y;
                tmp_df(_i, COLS::ITER) = float(t);
                _i++;  // maximum 4 iters
            }
        }

        // sample
        for (int i = 0; i < 4; i++) {
            float x = tmp_df(i, COLS::X), y = tmp_df(i, COLS::Y);
            tmp_df(i, COLS::OUTCOME) = sample(x, y);
        }
//        std::cout << "iter" << t << ", tmp_df:\n" << tmp_df << std::endl;

        // linear pipe fit()
        Eigen::MatrixX2f X = tmp_df(Eigen::all,
                                    Eigen::seq(COLS::X, COLS::Y));
        Eigen::VectorXf y = tmp_df.col(COLS::OUTCOME);
//        std::cout << "===polyFit===\nX:\n" << X << "\ny:\n" << y << std::endl;
        polyFit(X, y, 1);

        // next step
        float mean_x = tmp_df.col(COLS::X).mean();
        float mean_y = tmp_df.col(COLS::Y).mean();
        float range_x = tmp_df.col(COLS::X).maxCoeff() - tmp_df.col(COLS::X).minCoeff();
        float range_y = tmp_df.col(COLS::Y).maxCoeff() - tmp_df.col(COLS::Y).minCoeff();

        float coef_x = w[0];
        float coef_y = w[1];
        float ratio = coef_x / (coef_y + 1e-8f);
//        std::cout << "coef x = " << coef_x << ", coef y = " << coef_y << std::endl;

        float _increment_x = abs(ratio) * increment_y * (coef_x > 0 ? 1.f : -1.f);
        float _increment_y = increment_y * (coef_y > 0 ? 1.f : -1.f);
//        std::cout << "increament x = " << _increment_x << ", increament y = " << _increment_y << std::endl;
        float base_x = _increment_x * (range_x / 2) + mean_x;
        float base_y = _increment_y * (range_y / 2) + mean_y;
//        std::cout << "new x = " << base_x << ", new y = " << base_y << std::endl;
        // update x_t, y_t next doe
        x_t[0] = base_x - step_x;
        x_t[1] = base_x + step_x;
        y_t[0] = base_y - step_y;
        y_t[1] = base_y + step_y;

        // update step_df
        appendMat(step_df, tmp_df);
        float max_outcome = tmp_df.col(COLS::OUTCOME).maxCoeff();

        if (max_outcome < step_df.col(COLS::OUTCOME).maxCoeff()) {
            break;
            // TODO: add final step
        }
    }
}

void RSM::polyFit(const Eigen::MatrixX2f& X,
             const Eigen::VectorXf& y,
             int degree) {
    Eigen::MatrixXf Trans;  // Transform
    int N = static_cast<int>(X.rows());
    assert(degree > 0 && degree < 3);
    if (degree == 1) {
        Trans = Eigen::MatrixXf(N, 3);
        for (int i = 0; i < N; i++) {
            Trans(i, 0) = X(i, 0);
            Trans(i, 1) = X(i, 1);
            Trans(i, 2) = 1.0f;
        }
    }
    if (degree == 2) {
        Trans = Eigen::MatrixXf(N, 4);
        for (int i = 0; i < N; ++i) {
            Trans(i, 0) = X(i, 0);
            Trans(i, 1) = X(i, 1);
            Trans(i, 2) = X(i, 0) * X(i, 1);
            Trans(i, 3) = 1.0f;
        }
    }
    w = Trans.householderQr().solve(y);  // fast, not accurate
//    w = Trans.colPivHouseholderQr().solve(y);  // slow, accurate
//    w = Trans.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(y);
//    w = pinv_eigen_based(Trans) * y;
//    std::cout << "fitting::X:\n" << Trans << "\ny:\n" << y << "\nw:\n" << w << std::endl;
}

Eigen::VectorXf RSM::predict(const Eigen::MatrixX2f& X) const {
    Eigen::MatrixXf X_(X);
    X_.conservativeResize(X.rows(), X.cols()+1);
    X_(Eigen::all, X_.cols()-1) = Eigen::VectorXf::Ones(X_.rows());
    return X_ * w;
}

float sample_impl_test(float x, float y) {
    double x0, y0, fwhm;
    x0 = 0.08324215259452938;
    y0 = 0.5832421525945294;
    fwhm = 1.236124978496594;

    double func = 10.f * exp(-4*log(2) * (pow(x-x0, 2) + pow(y-y0, 2)) / pow(fwhm, 2));

    return float(func);
}

float sample_impl(float x, float y) {
    // TODO: change sample to error func
}

float RSM::sample(float x, float y) {
    return sample_impl_test(x, y);
}


