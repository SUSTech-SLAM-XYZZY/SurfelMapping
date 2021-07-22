//
// Created by zhijun on 2021/5/23.
//

#ifndef SURFELMAPPING_SURFELMAPPING_H
#define SURFELMAPPING_SURFELMAPPING_H

#include "Config.h"
#include "IndexMap.h"
#include "GlobalModel.h"
#include "Shaders.h"
#include "GPUTexture.h"
#include "FillIn.h"
#include "FeedbackBuffer.h"
#include "ComputePack.h"
#include "Checker.h"


class SurfelMapping
{
public:
    SurfelMapping();

    virtual ~SurfelMapping();

    /**
     * Process an rgb/depth map pair
     * @param rgb unsigned char row major order
     * @param depth unsigned short z-depth in millimeters, invalid depths are 0
     * @param gtPose optional input SE3 pose (if provided, we don't attempt to perform tracking)
     */
    void processFrame(const unsigned char * rgb,
                      const unsigned short * depth = nullptr,
                      const unsigned char * semantic = nullptr,
                      const Eigen::Matrix4f * gtPose = 0);

    /**
     * Predicts the current view of the scene, updates the [vertex/normal/image]Tex() members
     * of the indexMap class
     */
    void predict();

    /**
     * This class contains all of the predicted renders
     * @return reference
     */
    IndexMap & getIndexMap();

    /**
     * This class contains the surfel map
     * @return
     */
    GlobalModel & getGlobalModel();

    /**
     * @return the specific GPUTexture with name @param
     */
    pangolin::GlTexture * getTexture(const std::string &textureType);

    /**
     * @return the specific FeedbackBuffer with name @param
     */
    FeedbackBuffer * getFeedbackBuffer(const std::string &feedbackType);

    /**
     * The current global camera pose estimate
     * @return SE3 pose
     */
    const Eigen::Matrix4f & getCurrPose();



    /**
     * Calculate the above for the current frame (only done on the first frame normally)
     */
    void computeFeedbackBuffers();

    /**
     * Saves out a .ply mesh file of the current model
     */
    void savePly();

    /**
     * Renders a normalised view of the input raw depth for displaying as an OpenGL texture
     * (this is stored under texturefs[GPUTexture::DEPTH_NORM]
     * @param minVal minimum depth value to render
     * @param maxVal maximum depth value to render
     */
    void normaliseDepth(const float & minVal, const float & maxVal);


    Checker * checker;


private:
    int tick;
    Eigen::Matrix4f currPose;

    IndexMap indexMap;
    GlobalModel globalModel;
//    FillIn fillIn;

    std::map<std::string, GPUTexture*> textures;
    std::map<std::string, ComputePack*> computePacks;
    std::map<std::string, FeedbackBuffer*> feedbackBuffers;

    float nearClipDepth;
    float farClipDepth;


    void createTextures();
    void createCompute();
    void createFeedbackBuffers();

    // pre-processing methods
    void filterDepth();
    void metriciseDepth();
};

#endif //SURFELMAPPING_SURFELMAPPING_H