//
// Created by inkfin on 23-1-8.
//
#ifndef SURFELMAPPING_RSM_H
#define SURFELMAPPING_RSM_H

#include <iostream>
#include <utility>
#include <vector>
#include <cmath>
#include <iomanip>
#include <eigen3/Eigen/QR>
#include <eigen3/Eigen/SVD>
#include <eigen3/Eigen/Core>

/**
 * An simple RSM class implemented in Eigen
 *
 * @example @code
 * Eigen::Vector2f x_t = {0.5, 1};
 * Eigen::Vector2f y_t = {-0.5, -1};
 * RSM rsm(x_t, y_t);
 * Eigen::MatrixX4f step_df;
 * rsm.run(step_df);
 */
class RSM {
private:
    Eigen::Matrix4Xf df;  // (x, y, outcome, iter)

    Eigen::Vector2f x_t;  // x samples
    Eigen::Vector2f y_t;  // y samples

    Eigen::VectorXf w;

    /**
     * Polynomial regression
     *
     * (degree 1 or 2)
     * @param X (x, y), size=(n, 2)
     * @param y target, size=(n, 1)
     * @param degree polynomial degree, 1 or 2
     */
    void polyFit(const Eigen::MatrixX2f& X,
                 const Eigen::VectorXf& y,
                 int degree);

    /**
     * Polynomial regression model fitting
     * @param X (x, y), shape=(n, 2)
     * @return y predicted
     */
    Eigen::VectorXf predict(const Eigen::MatrixX2f& X) const;

    static float sample(float x, float y);


    // Utilities

    static void appendMat(Eigen::MatrixX4f& mat1, const Eigen::Matrix4f& mat2);

    static Eigen::MatrixXf pinv_eigen_based(const Eigen::MatrixXf& origin, float er = 0);

public:
    /**
     * Columns read from df
     * @example @code
     * df.col(COLS::OUTCOME)
     */
    enum COLS {
        X,
        Y,
        OUTCOME,
        ITER
    };

    int max_iter = 10;          // maximum iterations

    float increment_y = 2;      // increment y between each steps
    float step_x = 0.125;       // sample step
    float step_y = 0.125;


    RSM(Eigen::Vector2f x_t, Eigen::Vector2f y_t);

    /**
     * Run RSM tuning <br/>
     * It will update object's df with new samples
     */
    void run(Eigen::MatrixX4f& step_df);



};

#endif //SURFELMAPPING_RSM_H