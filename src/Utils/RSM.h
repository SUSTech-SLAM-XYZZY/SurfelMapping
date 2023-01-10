//
// Created by inkfin on 23-1-8.
//
#ifndef SURFELMAPPING_RSM_H
#define SURFELMAPPING_RSM_H

#include <eigen3/Eigen/Core>

class GlobalModel;

/**
 * An simple RSM class implemented in Eigen
 *
 * @example @code
 * Eigen::Vector2f x_t = {0.5, 1};
 * Eigen::Vector2f y_t = {-0.5, -1};
 * RSM rsm();
 * rsm.x_t = x_t;
 * rsm.y_t = y_t;
 * rsm.max_iter = 10;
 * rsm.increment_y_l = 2;
 * rsm.increment_y_s = 1;
 * rsm.step_x = 0.125;
 * rsm.step_y = 0.125;
 * rsm.clear();
 * rsm.run();
 */
class RSM {
private:

    RSM() {}

    Eigen::VectorXf w;

    void nextStep(const Eigen::MatrixX4f& tmp_df, float increment_y);

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

    static float sample_test(float x, float y);

    float sample(float x, float y);

    Eigen::Vector4f optim_sample;

    GlobalModel* globalModel;


    // Utilities

    static void appendMat(Eigen::MatrixX4f& mat1, const Eigen::MatrixX4f& mat2);

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

    Eigen::MatrixX4f step_df;   // (x, y, outcome, iter)

    Eigen::Vector2f x_t;  // x doe coordinates
    Eigen::Vector2f y_t;  // y doe coordinates

    int max_iter = 10;          // maximum iterations

    float increment_y_l = 2;    // increment y long between each steps
    float increment_y_s = 1;    // increment y short in final stage
    float step_x = 0.125;       // sample step
    float step_y = 0.125;


    RSM(GlobalModel* globalModel): globalModel(globalModel) {}

    /**
     * Run RSM tuning <br/>
     * It will update object's df with new samples
     */
    void run();

    /**
     * Get optimal sample after run(), use COLS to get items
     * @return A vector contains maximum sample infos
     * @example @code
     * Vector4f optim = rsm.get_optimal_sample();
     * float X = optim(COLS::X)
     * float Y = optim(COLS::Y)
     * float OUTCOME = optim(COLS::OUTCOME)
     * float ITER = optim(COLS::ITER)
     */
    const Eigen::Vector4f& get_optimal_sample();

    void clear();

};

#endif //SURFELMAPPING_RSM_H