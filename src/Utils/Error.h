//
// Created by bill on 1/7/23.
//

#ifndef SURFELMAPPING_ERROR_H
#define SURFELMAPPING_ERROR_H

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

extern double getPSNR(const cv::Mat& I1, const cv::Mat& I2);

extern cv::Scalar getMSSIM( const cv::Mat& i1, const cv::Mat& i2);

#endif //SURFELMAPPING_ERROR_H
