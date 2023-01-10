

#include "GlobalModel.h"

GlobalModel::GlobalModel()
 : TEXTURE_DIMENSION(Config::maxSqrtVertices()),
   MAX_VERTICES(TEXTURE_DIMENSION * TEXTURE_DIMENSION),
   bufferSize(MAX_VERTICES * Config::vertexSize()),
   count(0),
   offset(0),
   dataCount(0),
   conflictCount(0),
   unstableCount(0),
   renderCount(0),
   tmpCount(0),
   initProgram(loadProgramFromFile("init_unstable.vert")),
   modelProgram(loadProgramFromFile("map.vert", "map.frag")),
   dataProgram(loadProgramGeomFromFile("data.vert", "data.geom")),
   genLSMProgram(loadProgramGeomFromFile("genLSM.vert", "genLSM.geom")),
   getGSMViewProgram(loadProgramGeomFromFile("gsmView.vert", "gsmView.geom")),
   conflictProgram(loadProgramGeomFromFile("conflict.vert", "conflict.geom")),
   fuseProgram(loadProgramFromFile("fuse.vert", "fuse.frag")),
   updateConflictProgram(loadProgramFromFile("update_conf.vert", "update_conf.frag")),
   backMappingProgram(loadProgramGeomFromFile("back_map.vert", "back_map.geom")),
   unstableProgram(loadProgramGeomFromFile("unstable.vert", "unstable.geom")),
   adaptiveRenderToBufferProgram(loadProgramGeomFromFile("adaptive_rendering.vert", "adaptive_rendering.geom")),
   drawPointProgram(loadProgramFromFile("draw_feedback.vert", "draw_feedback.frag")),
   drawSurfelProgram(loadProgramFromFile("draw_surface.vert", "draw_surface_adaptive.geom", "draw_surface.frag")),
   drawImageProgram(loadProgramFromFile("draw_image.vert", "draw_image_adaptive.geom", "draw_image.frag")),
   renderImageToBufferProgram(loadProgramFromFile("draw_image.vert", "draw_image.geom", "draw_image.frag")),
   //drawImageProgram(loadProgramFromFile("empty.vert", "quad.geom", "draw_image.frag")),
   modelMapRenderBuffer(TEXTURE_DIMENSION, TEXTURE_DIMENSION),
   vertConfRenderBuffer(TEXTURE_DIMENSION, TEXTURE_DIMENSION),
   modelMapVertsConfs(TEXTURE_DIMENSION, TEXTURE_DIMENSION, GL_RGBA32F, GL_RED, GL_FLOAT),
   modelMapColorsTime(TEXTURE_DIMENSION, TEXTURE_DIMENSION, GL_RGBA32F, GL_RED, GL_FLOAT),
   modelMapNormsRadii(TEXTURE_DIMENSION, TEXTURE_DIMENSION, GL_RGBA32F, GL_RED, GL_FLOAT)
{
    glGenTransformFeedbacks(1, &modelFid);
    glGenBuffers(1, &modelVbo);
    glBindBuffer(GL_ARRAY_BUFFER, modelVbo);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_DYNAMIC_COPY);  // only allocate space
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTransformFeedbacks(1, &renderFid);
    glGenBuffers(1, &renderVbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderVbo);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_DYNAMIC_COPY);  // only allocate space
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTransformFeedbacks(1, &tmpFid);
    glGenBuffers(1, &tmpVbo);
    glBindBuffer(GL_ARRAY_BUFFER, tmpVbo);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_DYNAMIC_COPY);  // only allocate space
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTransformFeedbacks(1, &dataFid);
    glGenBuffers(1, &dataVbo);
    glBindBuffer(GL_ARRAY_BUFFER, dataVbo);
    glBufferData(GL_ARRAY_BUFFER, Config::numPixels() * Config::vertexSize(), nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTransformFeedbacks(1, &lsmFid);
    glGenBuffers(1, &lsmVbo);
    glBindBuffer(GL_ARRAY_BUFFER, lsmVbo);
    glBufferData(GL_ARRAY_BUFFER, Config::numPixels() * Config::vertexSize(), nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTransformFeedbacks(1, &gsmViewFid);
    glGenBuffers(1, &gsmViewVbo);
    glBindBuffer(GL_ARRAY_BUFFER, gsmViewVbo);
    glBufferData(GL_ARRAY_BUFFER, Config::numPixels() * Config::vertexSize(), nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenTransformFeedbacks(1, &conflictFid);
    glGenBuffers(1, &conflictVbo);
    glBindBuffer(GL_ARRAY_BUFFER, conflictVbo);
    glBufferData(GL_ARRAY_BUFFER,
                 Config::numPixels() * (sizeof(float) + (int)(Config::vertexSize() / 3)), // id + posConf
                 nullptr,
                 GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &unstableVbo);
    glBindBuffer(GL_ARRAY_BUFFER, unstableVbo);
    glBufferData(GL_ARRAY_BUFFER, Config::numPixels() * Config::vertexSize(), nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    std::vector<Eigen::Vector2f> uv;
    uv.reserve(Config::numPixels());
    for(int i = 0; i < Config::W(); ++i)
    {
        for(int j = 0; j < Config::H(); ++j)
        {
            uv.push_back(Eigen::Vector2f((i + 0.5) / (float)Config::W(),
                                         (j + 0.5) / (float)Config::H() ) );
        }
    }
    uvSize = uv.size();
    glGenBuffers(1, &uvo);
    glBindBuffer(GL_ARRAY_BUFFER, uvo);
    glBufferData(GL_ARRAY_BUFFER, uvSize * sizeof(Eigen::Vector2f), &uv[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    uv.clear();
    uv.reserve(MAX_VERTICES);
    /// big uvo must be row major to fit the model map
    for(int j = 0; j < TEXTURE_DIMENSION; ++j)
    {
        for(int i = 0; i < TEXTURE_DIMENSION; ++i)
        {
            uv.push_back(Eigen::Vector2f(((float)i / (float)TEXTURE_DIMENSION) + 1.0 / (2 * (float)TEXTURE_DIMENSION),
                                         ((float)j / (float)TEXTURE_DIMENSION) + 1.0 / (2 * (float)TEXTURE_DIMENSION)));
        }
    }
    bigUvSize = uv.size();
    glGenBuffers(1, &bigUvo);
    glBindBuffer(GL_ARRAY_BUFFER, bigUvo);
    glBufferData(GL_ARRAY_BUFFER, bigUvSize * sizeof(Eigen::Vector2f), &uv[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    modelMapFramebuffer.AttachColour(*modelMapVertsConfs.texture);
    modelMapFramebuffer.AttachColour(*modelMapColorsTime.texture);
    modelMapFramebuffer.AttachColour(*modelMapNormsRadii.texture);
    modelMapFramebuffer.AttachDepth(modelMapRenderBuffer);

    vertConfFrameBuffer.AttachColour(*modelMapVertsConfs.texture);
    vertConfFrameBuffer.AttachDepth(modelMapRenderBuffer);

//    setImageSize(Config::W(), Config::H(), Config::fx(), Config::fy(), Config::cx(), Config::cy());
//    imageFramebuffer.AttachColour(imageTexture);
//    imageFramebuffer.AttachColour(semanticTexture);
//    imageFramebuffer.AttachColour(depthTexture);
//    imageFramebuffer.AttachDepth(imageRenderBuffer);

    //------------- specify the retrieved varyings
    initProgram->Bind();
    int locInit[3] =
            {
                    glGetVaryingLocationNV(initProgram->programId(), "vPosition0"),
                    glGetVaryingLocationNV(initProgram->programId(), "vColor0"),
                    glGetVaryingLocationNV(initProgram->programId(), "vNormRad0")
            };
    glTransformFeedbackVaryingsNV(initProgram->programId(), 3, locInit, GL_INTERLEAVED_ATTRIBS);
    initProgram->Unbind();

    dataProgram->Bind();
    int dataUpdate[3] =
            {
                    glGetVaryingLocationNV(dataProgram->programId(), "vPosition0"),
                    glGetVaryingLocationNV(dataProgram->programId(), "vColor0"),
                    glGetVaryingLocationNV(dataProgram->programId(), "vNormRad0")
            };
    glTransformFeedbackVaryingsNV(dataProgram->programId(), 3, dataUpdate, GL_INTERLEAVED_ATTRIBS);
    dataProgram->Unbind();

    genLSMProgram->Bind();
    int lsmModule[3] =
            {
                    glGetVaryingLocationNV(genLSMProgram->programId(), "vPosition0"),
                    glGetVaryingLocationNV(genLSMProgram->programId(), "vColor0"),
                    glGetVaryingLocationNV(genLSMProgram->programId(), "vNormRad0")
            };
    glTransformFeedbackVaryingsNV(genLSMProgram->programId(), 3, lsmModule, GL_INTERLEAVED_ATTRIBS);
    genLSMProgram->Unbind();

    getGSMViewProgram->Bind();
    int gsmViewModule[3] =
            {
                    glGetVaryingLocationNV(getGSMViewProgram->programId(), "vPosition0"),
                    glGetVaryingLocationNV(getGSMViewProgram->programId(), "vColor0"),
                    glGetVaryingLocationNV(getGSMViewProgram->programId(), "vNormRad0")
            };
    glTransformFeedbackVaryingsNV(getGSMViewProgram->programId(), 3, gsmViewModule, GL_INTERLEAVED_ATTRIBS);
    getGSMViewProgram->Unbind();

    conflictProgram->Bind();
    int conflictUpdate[2] =
            {
                    glGetVaryingLocationNV(conflictProgram->programId(), "conflictId0"),
                    glGetVaryingLocationNV(conflictProgram->programId(), "vPosition0")
            };
    glTransformFeedbackVaryingsNV(conflictProgram->programId(), 2, conflictUpdate, GL_INTERLEAVED_ATTRIBS);
    conflictProgram->Unbind();

    backMappingProgram->Bind();
    int locUpdate[3] =
    {
        glGetVaryingLocationNV(backMappingProgram->programId(), "vPosition0"),
        glGetVaryingLocationNV(backMappingProgram->programId(), "vColorTime0"),
        glGetVaryingLocationNV(backMappingProgram->programId(), "vNormRad0"),
    };
    glTransformFeedbackVaryingsNV(backMappingProgram->programId(), 3, locUpdate, GL_INTERLEAVED_ATTRIBS);
    backMappingProgram->Unbind();

    unstableProgram->Bind();
    int unstableUpdate[3] =
    {
        glGetVaryingLocationNV(unstableProgram->programId(), "vPosition0"),
        glGetVaryingLocationNV(unstableProgram->programId(), "vColorTime0"),
        glGetVaryingLocationNV(unstableProgram->programId(), "vNormRad0"),
    };
    glTransformFeedbackVaryingsNV(unstableProgram->programId(), 3, unstableUpdate, GL_INTERLEAVED_ATTRIBS);
    unstableProgram->Unbind();

    adaptiveRenderToBufferProgram->Bind();
    int adaptiveRendering[3] =
            {
                    glGetVaryingLocationNV(dataProgram->programId(), "vPosition0"),
                    glGetVaryingLocationNV(dataProgram->programId(), "vColor0"),
                    glGetVaryingLocationNV(dataProgram->programId(), "vNormRad0")
            };
    glTransformFeedbackVaryingsNV(adaptiveRenderToBufferProgram->programId(), 3, adaptiveRendering, GL_INTERLEAVED_ATTRIBS);
    adaptiveRenderToBufferProgram->Unbind();


    glGenQueries(1, &countQuery);

    CheckGlDieOnError();
}

GlobalModel::~GlobalModel()
{
    glDeleteTransformFeedbacks(1, &modelFid);
    glDeleteBuffers(1, &modelVbo);

    glDeleteTransformFeedbacks(1, &dataFid);
    glDeleteBuffers(1, &dataVbo);

    glDeleteTransformFeedbacks(1, &lsmFid);
    glDeleteBuffers(1, &lsmVbo);

    glDeleteTransformFeedbacks(1, &gsmViewFid);
    glDeleteBuffers(1, &gsmViewVbo);

    glDeleteTransformFeedbacks(1, &renderFid);
    glDeleteBuffers(1, &renderVbo);

    glDeleteQueries(1, &countQuery);

    glDeleteBuffers(1, &uvo);
}

void GlobalModel::initialize(const FeedbackBuffer & rawFeedback, const Eigen::Matrix4f &pose)
{
    // just simply copy
    initProgram->Bind();

    initProgram->setUniform(Uniform("pose", pose));

    glBindBuffer(GL_ARRAY_BUFFER, rawFeedback.vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    glEnable(GL_RASTERIZER_DISCARD);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, modelFid);

    //glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, modelVbo);
    glTransformFeedbackBufferBase(modelFid, 0, modelVbo);

    glBeginTransformFeedback(GL_POINTS);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, countQuery);

    //It's ok to use either fid because both raw and filtered have the same amount of vertices
    glDrawTransformFeedback(GL_POINTS, rawFeedback.fid);

    glEndTransformFeedback();

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    glGetQueryObjectuiv(countQuery, GL_QUERY_RESULT, &count);

    glDisable(GL_RASTERIZER_DISCARD);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    initProgram->Unbind();

    glFinish();


    CheckGlDieOnError();
}

void GlobalModel::dataAssociate(const Eigen::Matrix4f &pose,
                                const int &time,
                                GPUTexture *rgb,
                                GPUTexture *depthRaw,
                                GPUTexture *semantic,
                                GPUTexture *indexMap,
                                GPUTexture *vertConfMap,
                                GPUTexture *colorTimeMap,
                                GPUTexture *normRadMap,
                                const float depthMin,
                                const float depthMax)
{
    TICK("Data::Association");

    //This part computes new vertices and the vertices to merge with, storing
    //in an array that sets which vertices to update by index
    dataProgram->Bind();

    dataProgram->setUniform(Uniform("cSampler", 0));
    dataProgram->setUniform(Uniform("drSampler", 1));
    dataProgram->setUniform(Uniform("sSampler", 2));
    dataProgram->setUniform(Uniform("indexSampler", 3));
    dataProgram->setUniform(Uniform("vertConfSampler", 4));
    dataProgram->setUniform(Uniform("colorTimeSampler", 5));
    dataProgram->setUniform(Uniform("normRadSampler", 6));
    dataProgram->setUniform(Uniform("time", (float)time));

    dataProgram->setUniform(Uniform("cam", Eigen::Vector4f(Config::cx(),
                                                           Config::cy(),
                                                           1.0 / Config::fx(),
                                                           1.0 / Config::fy())));
    dataProgram->setUniform(Uniform("cols", (float)Config::W()));
    dataProgram->setUniform(Uniform("rows", (float)Config::H()));
    dataProgram->setUniform(Uniform("scale", IndexMap::FACTOR));
    dataProgram->setUniform(Uniform("texDim", (float)TEXTURE_DIMENSION));
    dataProgram->setUniform(Uniform("pose", pose));
    dataProgram->setUniform(Uniform("minDepth", depthMin));
    dataProgram->setUniform(Uniform("maxDepth", depthMax));
    dataProgram->setUniform(Uniform("fuseThresh", Config::surfelFuseDistanceThreshFactor()));

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, uvo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnable(GL_RASTERIZER_DISCARD);
    // use GPU to accerlate the progress
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, dataFid);
    //glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, dataVbo);
    glTransformFeedbackBufferBase(dataFid, 0, dataVbo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rgb->texture->tid);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthRaw->texture->tid);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, semantic->texture->tid);
    // pixel -> Surfel (indexMap)
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, indexMap->texture->tid);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, vertConfMap->texture->tid);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, colorTimeMap->texture->tid);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, normRadMap->texture->tid);

    glBeginTransformFeedback(GL_POINTS);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, countQuery);
    dataCount = 0;

    glDrawArrays(GL_POINTS, 0, uvSize);

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glGetQueryObjectuiv(countQuery, GL_QUERY_RESULT, &dataCount);

    glEndTransformFeedback();

    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    glDisable(GL_RASTERIZER_DISCARD);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    dataProgram->Unbind();

    glFinish();

    TOCK("Data::Association");

    CheckGlDieOnError()
}

void GlobalModel::updateFuse()
{
    TICK("Update::Fuse");

    // Change model map - I
    modelMapFramebuffer.Bind();

    glPushAttrib(GL_VIEWPORT_BIT);

    glViewport(0, 0, modelMapRenderBuffer.width, modelMapRenderBuffer.height);

    glDisable(GL_DEPTH_TEST);

    fuseProgram->Bind();
    fuseProgram->setUniform(Uniform("texDim", TEXTURE_DIMENSION));

    glBindBuffer(GL_ARRAY_BUFFER, dataVbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    //glDrawTransformFeedback(GL_POINTS, dataFid);
    glDrawArrays(GL_POINTS, 0, dataCount);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    fuseProgram->Unbind();

    glEnable(GL_DEPTH_TEST);

    glPopAttrib();

    modelMapFramebuffer.Unbind();

    glFinish();

    TOCK("Update::Fuse");

    CheckGlDieOnError();
}

void GlobalModel::processConflict(const Eigen::Matrix4f &pose,
                                  const int &time,
                                  GPUTexture *depthRaw,
                                  GPUTexture * semantic,
                                  float minDepth,
                                  float maxDepth,
                                  float fuseThresh,
                                  int isClean)
{
    conflictProgram->Bind();

    conflictProgram->setUniform(Uniform("drSampler", 0));
    conflictProgram->setUniform(Uniform("seSampler", 1));

    conflictProgram->setUniform(Uniform("cam", Eigen::Vector4f(Config::cx(),
                                                               Config::cy(),
                                                               Config::fx(),
                                                               Config::fy())));
    conflictProgram->setUniform(Uniform("cols", (float)Config::W()));
    conflictProgram->setUniform(Uniform("rows", (float)Config::H()));
    conflictProgram->setUniform(Uniform("minDepth", minDepth));
    conflictProgram->setUniform(Uniform("maxDepth", maxDepth));

    Eigen::Matrix4f t_inv = pose.inverse();  // T_w^c
    conflictProgram->setUniform(Uniform("t_inv", t_inv));
    conflictProgram->setUniform(Uniform("fuseThresh", fuseThresh));
    conflictProgram->setUniform(Uniform("stereoBorder", 80.f));
    conflictProgram->setUniform(Uniform("isClean", isClean));

    glBindBuffer(GL_ARRAY_BUFFER, modelVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 1));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    glEnable(GL_RASTERIZER_DISCARD);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, conflictFid);

    //glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, conflictVbo);
    glTransformFeedbackBufferBase(conflictFid, 0, conflictVbo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthRaw->texture->tid);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, semantic->texture->tid);

    glBeginTransformFeedback(GL_POINTS);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, countQuery);
    conflictCount = 0;

    glDrawArrays(GL_POINTS, 0, count);

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glGetQueryObjectuiv(countQuery, GL_QUERY_RESULT, &conflictCount);

    glEndTransformFeedback();

    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    glDisable(GL_RASTERIZER_DISCARD);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    conflictProgram->Unbind();

    glFinish();

    CheckGlDieOnError();

}

void GlobalModel::updateConflict()
{
    vertConfFrameBuffer.Bind();

    glPushAttrib(GL_VIEWPORT_BIT);

    glViewport(0, 0, vertConfRenderBuffer.width, vertConfRenderBuffer.height);

    glDisable(GL_DEPTH_TEST);

    updateConflictProgram->Bind();
    updateConflictProgram->setUniform(Uniform("texDim", TEXTURE_DIMENSION));

    glBindBuffer(GL_ARRAY_BUFFER, conflictVbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float) + (int)(Config::vertexSize() / 3), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) + (int)(Config::vertexSize() / 3), reinterpret_cast<GLvoid*>(sizeof(float)));

    glDrawArrays(GL_POINTS, 0, conflictCount);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    updateConflictProgram->Unbind();

    glEnable(GL_DEPTH_TEST);

    glPopAttrib();

    vertConfFrameBuffer.Unbind();

    glFinish();

    CheckGlDieOnError()
}

void GlobalModel::backMapping()
{
    TICK("Back::Clean");

    // First we copy modelMap* texturefs to modelVbo
    backMappingProgram->Bind();
    backMappingProgram->setUniform(Uniform("vertConfSampler", 0));
    backMappingProgram->setUniform(Uniform("colorTimeSampler", 1));
    backMappingProgram->setUniform(Uniform("normRadSampler", 2));

    glEnable(GL_RASTERIZER_DISCARD);

    // set receiver
    //glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, modelVbo);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, modelFid);
    glTransformFeedbackBufferBase(modelFid, 0, modelVbo);

    glBeginTransformFeedback(GL_POINTS);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, countQuery);
    offset = 0;

    glBindBuffer(GL_ARRAY_BUFFER, bigUvo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, modelMapVertsConfs.texture->tid);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, modelMapColorsTime.texture->tid);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, modelMapNormsRadii.texture->tid);

    glDrawArrays(GL_POINTS, 0, count);   // only partial pixels are valid

    glEndTransformFeedback();

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glGetQueryObjectuiv(countQuery, GL_QUERY_RESULT, &offset);

    glDisable(GL_RASTERIZER_DISCARD);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    backMappingProgram->Unbind();

    glFinish();

    TOCK("Back::Clean");

    count = offset;


    CheckGlDieOnError();
}

void GlobalModel::concatenate()
{
    TICK("Concatenate");

    unstableProgram->Bind();

    glEnable(GL_RASTERIZER_DISCARD);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, unstableVbo);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, countQuery);
    unstableCount = 0;

    glBeginTransformFeedback(GL_POINTS);

    glBindBuffer(GL_ARRAY_BUFFER, dataVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    glDrawArrays(GL_POINTS, 0, dataCount);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEndTransformFeedback();

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glGetQueryObjectuiv(countQuery, GL_QUERY_RESULT, &unstableCount);

    glDisable(GL_RASTERIZER_DISCARD);

    unstableProgram->Unbind();

    glFinish();

    //-----------------------------------------------------

    glBindBuffer(GL_COPY_READ_BUFFER, unstableVbo);
    glBindBuffer(GL_COPY_WRITE_BUFFER, modelVbo);

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset * Config::vertexSize(), unstableCount * Config::vertexSize());

    count = offset + unstableCount;

    glFinish();

    TOCK("Concatenate");


    CheckGlDieOnError();
}

void GlobalModel::buildModelMap()
{
    modelMapFramebuffer.Bind();

    glPushAttrib(GL_VIEWPORT_BIT);

    glViewport(0, 0, modelMapRenderBuffer.width, modelMapRenderBuffer.height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelProgram->Bind();

    modelProgram->setUniform(Uniform("texDim", TEXTURE_DIMENSION));

    glBindBuffer(GL_ARRAY_BUFFER, modelVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 1));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    //glDrawTransformFeedback(GL_POINTS, modelFid);
    glDrawArrays(GL_POINTS, 0, count);

    modelMapFramebuffer.Unbind();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // build model map each time modelVbo is updated
    modelProgram->Unbind();

    glPopAttrib();

    glFinish();


    CheckGlDieOnError();
}

void GlobalModel::renderModel(pangolin::OpenGlMatrix mvp,
                              pangolin::OpenGlMatrix mv,
                              const float threshold,
                              const bool drawUnstable,
                              const bool drawNormals,
                              const bool drawColors,
                              const bool drawPoints,
                              const bool drawWindow,
                              const bool drawSemantic,
                              const int time,
                              const int timeDelta)
{
    std::shared_ptr<Shader> program = drawPoints ? drawPointProgram : drawSurfelProgram;

    program->Bind();

    program->setUniform(Uniform("MVP", mvp));

    program->setUniform(Uniform("MVINV", mv.Inverse()));

    program->setUniform(Uniform("threshold", threshold));

    program->setUniform(Uniform("colorType", (drawNormals ? 1 : drawColors ? 2 : drawSemantic ? 3 : 0)));

    program->setUniform(Uniform("unstable", drawUnstable));

    program->setUniform(Uniform("drawWindow", drawWindow));

    program->setUniform(Uniform("time", time));

    program->setUniform(Uniform("timeDelta", timeDelta));

    Eigen::Matrix4f pose = Eigen::Matrix4f::Identity();
    program->setUniform(Uniform("pose", pose));

    program->setUniform(Uniform("class0", Eigen::Vector3f(128,128,128)));
    program->setUniform(Uniform("class1", Eigen::Vector3f(0,255,0)));
    program->setUniform(Uniform("class2", Eigen::Vector3f(0,0,255)));
    program->setUniform(Uniform("class3", Eigen::Vector3f(255,255,0)));
    program->setUniform(Uniform("class4", Eigen::Vector3f(128,0,0)));
    program->setUniform(Uniform("class5", Eigen::Vector3f(255,0,255)));
    program->setUniform(Uniform("class6", Eigen::Vector3f(128,128,0)));
    program->setUniform(Uniform("class7", Eigen::Vector3f(0,128,0)));
    program->setUniform(Uniform("class8", Eigen::Vector3f(128,0,128)));
    program->setUniform(Uniform("class9", Eigen::Vector3f(0,128,128)));
    program->setUniform(Uniform("class10", Eigen::Vector3f(0,255,255)));
    program->setUniform(Uniform("class11", Eigen::Vector3f(0,0,128)));
    program->setUniform(Uniform("class12", Eigen::Vector3f(245,222,179)));
    program->setUniform(Uniform("class13", Eigen::Vector3f(255,0,0)));
    program->setUniform(Uniform("class14", Eigen::Vector3f(210,105,30)));
    program->setUniform(Uniform("class15", Eigen::Vector3f(244,164,96)));
    program->setUniform(Uniform("class16", Eigen::Vector3f(119,136,153)));
    program->setUniform(Uniform("class17", Eigen::Vector3f(255,20,147)));
    program->setUniform(Uniform("class18", Eigen::Vector3f(138,43,226)));

    glBindBuffer(GL_ARRAY_BUFFER, modelVbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 1));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    //glDrawTransformFeedback(GL_POINTS, modelFid);
    glDrawArrays(GL_POINTS, 0, count);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    program->Unbind();
}

void GlobalModel::resetBuffer()
{
    clearBuffer(modelVbo, 0.f);
    count = 0;
    clearBuffer(dataVbo, 0.f);
    dataCount = 0;
    clearBuffer(conflictVbo, 0.f);
    conflictCount = 0;
    clearBuffer(lsmVbo, 0.f);
    lsmcount = 0;
    clearBuffer(gsmViewVbo, 0.f);
    gsmViewcount = 0;
    clearBuffer(unstableVbo, 0.f);
    unstableCount = 0;
}

void GlobalModel::setImageSize(int w, int h, float fx, float fy, float cx, float cy)
{
    imageRenderBuffer.Reinitialise(w, h);
    imageTexture.Reinitialise(w, h, GL_RGB8UI, true, 0, GL_RGB_INTEGER, GL_UNSIGNED_BYTE);
    semanticTexture.Reinitialise(w, h, GL_R8UI, true, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE);
    //depthTexture.Reinitialise(w, h, GL_R16UI, true, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT);

    imageCam << cx, cy, fx, fy;
}

void GlobalModel::transformToBuffer(std::pair<GLuint, GLuint> source, std::pair<GLuint, GLuint> target){
    target.second = source.second;
    // load imageTexture  semanticTexture  depthTexture  imageRenderBuffer
    glBindBuffer(GL_COPY_READ_BUFFER, source.first);
    glBindBuffer(GL_COPY_WRITE_BUFFER, target.first);

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, source.second * Config::vertexSize());

    glFinish();

    CheckGlDieOnError()
}

void GlobalModel::rsmTuning(const Eigen::Matrix4f &view, int frameid) {
    float * vertexData = this->getVertexDataFromBuffer(this->renderVbo, renderCount);
    Eigen::MatrixXf vertex = Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>(vertexData, renderCount, 12);
    for(int i = 0; i < renderCount; i++){
        Eigen::Vector2f x_t = {1, 1};
        Eigen::Vector2f y_t = {0, 0};
        RSM rsm(this);
        rsm.x_t = x_t;
        rsm.y_t = y_t;
        rsm.max_iter = 100;
        rsm.increment_y_l = 5.0f;
        rsm.increment_y_s = 2.5f;
        rsm.step_x = 0.5;
        rsm.step_y = 0.5;
        rsm.frame_id = frameid;
        rsm.view = view;
        rsm.vertex_id = i;
        rsm.vertexData = vertex;
        rsm.run();
        auto ans = rsm.get_optimal_sample();
        // fetch out data
        auto readyTransVertex = vertex.row(i);
        Eigen::Vector4f new_normal;
        // trans the vertex due to x & y
//        this->rotateNormal(readyTransVertex.head(4), readyTransVertex.tail(4), new_normal, view, ans(RSM::COLS::X), ans(RSM::COLS::Y));
        std::cout << "=====1=====" << std::endl << new_normal.transpose() << std::endl;
        std::cout << "=====2=====" << std::endl << readyTransVertex.tail(4) << std::endl;
        std::cout << "=====3=====" << std::endl << ans.transpose() << std::endl;
        readyTransVertex.tail(4) = new_normal;
    }
    // transfrom it into renderBuffer
    this->clearBuffer(renderVbo, 0.0f);
    glBindBuffer(GL_ARRAY_BUFFER, renderVbo);
    glBufferData(GL_ARRAY_BUFFER, renderCount * Config::vertexSize(), vertex.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    free(vertexData);

    CheckGlDieOnError()
}

void GlobalModel::transfromRenderBuffer(float x, float y, int vertex_id, const Eigen::Matrix4f& view, Eigen::MatrixXf vertexData){
    auto readyTransVertex = vertexData.row(vertex_id);
    Eigen::Vector4f new_normal;
    // trans the vertex due to x & y
    this->rotateNormal(readyTransVertex.head(4), readyTransVertex.tail(4), new_normal, view, x, y);
    readyTransVertex.tail(4) = new_normal;
    // transfrom back
    glBindBuffer(GL_ARRAY_BUFFER, tmpVbo);
    glBufferData(GL_ARRAY_BUFFER, renderCount * Config::vertexSize(), vertexData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GlobalModel::rotateNormal(const Eigen::Vector4f& position,
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
    rot = Eigen::AngleAxisf(yaw/180.f * M_PI, y)
          * Eigen::AngleAxisf(pitch/180.f * M_PI, x);
    Eigen::Vector3f new_vNormRad = rot * vNormRad;

    Eigen::Vector3f new_normal_3f = t_inv.inverse() * new_vNormRad;
    new_normal.head<3>() = new_normal_3f;
    new_normal(3) = normal(3);
}

void GlobalModel::adptiveRenderToBuffer(const Eigen::Matrix4f &pose)
{
    adaptiveRenderToBufferProgram->Bind();

    Eigen::Matrix4f t_inv = pose.inverse();
    adaptiveRenderToBufferProgram->setUniform(Uniform("t_inv", t_inv));
    adaptiveRenderToBufferProgram->setUniform(Uniform("cam", imageCam));
    adaptiveRenderToBufferProgram->setUniform(Uniform("pose", pose));
    adaptiveRenderToBufferProgram->setUniform(Uniform("cols", (float)imageRenderBuffer.width));
    adaptiveRenderToBufferProgram->setUniform(Uniform("rows", (float)imageRenderBuffer.height));
    adaptiveRenderToBufferProgram->setUniform(Uniform("maxDepth", 200.f));
    adaptiveRenderToBufferProgram->setUniform(Uniform("threshold", 0.f));

    glBindBuffer(GL_ARRAY_BUFFER, modelVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 1));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    glEnable(GL_RASTERIZER_DISCARD);
    // use GPU to accerlate the progress
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, renderFid);
    //glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, dataVbo);
    glTransformFeedbackBufferBase(renderFid, 0, renderVbo);

    glBeginTransformFeedback(GL_POINTS);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, countQuery);
    renderCount = 0;

    glDrawArrays(GL_POINTS, 0, count);

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glGetQueryObjectuiv(countQuery, GL_QUERY_RESULT, &renderCount);

    glEndTransformFeedback();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    adaptiveRenderToBufferProgram->Unbind();

    glFinish();

    CheckGlDieOnError()
}

void GlobalModel::RenderingImageToTexture(const Eigen::Matrix4f &view, std::pair<GLuint, GLuint> Vbo){
    pangolin::GlFramebuffer imageFramebuffer;
    imageFramebuffer.AttachColour(imageTexture);
    imageFramebuffer.AttachColour(semanticTexture);
    //imageFramebuffer.AttachColour(depthTexture);
    imageFramebuffer.AttachDepth(imageRenderBuffer);

    imageFramebuffer.Bind();

    glPushAttrib(GL_VIEWPORT_BIT);

    glViewport(0, 0, imageRenderBuffer.width, imageRenderBuffer.height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderImageToBufferProgram->Bind();

    Eigen::Matrix4f t_inv = view.inverse();
    renderImageToBufferProgram->setUniform(Uniform("t_inv", t_inv));
    renderImageToBufferProgram->setUniform(Uniform("cam", imageCam));
    renderImageToBufferProgram->setUniform(Uniform("cols", (float)imageRenderBuffer.width));
    renderImageToBufferProgram->setUniform(Uniform("rows", (float)imageRenderBuffer.height));
    renderImageToBufferProgram->setUniform(Uniform("maxDepth", 200.f));
    renderImageToBufferProgram->setUniform(Uniform("threshold", 0.f));

    glBindBuffer(GL_ARRAY_BUFFER, Vbo.first);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 1));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    glDrawArrays(GL_POINTS, 0, Vbo.second);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    renderImageToBufferProgram->Unbind();

    glPopAttrib();

    imageFramebuffer.Unbind();

    glFinish();

    CheckGlDieOnError()
}

double GlobalModel::getRGBImgLoss(cv::Mat& paired_Img, int frame_id){
    std::string rgb_path = "/home/bill/prog/Surfel/dataset/image_2/";
    // calc the loss
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(6) << frame_id;
    std::string file_name = ss.str() + ".png";

    cv::Mat rgb = cv::imread(rgb_path + file_name, cv::IMREAD_COLOR);

//    std::cout << "Frame " << frame_id << std::endl;
//    std::cout << "PSNR = " << getPSNR(paired_Img, rgb) << std::endl;
//    std::cout << "SSIM = " << getMSSIM(paired_Img, rgb) << std::endl;

    return getPSNR(paired_Img, rgb);
}

double GlobalModel::getError(const Eigen::Matrix4f &view, int frameid){
    auto texturePtr = new unsigned char [Config::W() * Config::H() * 3];
    this->RenderingImageToTexture(view, this->getmpBuffer());
    this->getImageTex()->Download(texturePtr, GL_RGB_INTEGER, GL_UNSIGNED_BYTE);

    CheckGlDieOnError()

    cv::Mat image(Config::H(), Config::W(), CV_8UC3);
    memcpy(image.data, texturePtr, Config::W() * Config::H() * 3);

    delete(texturePtr);

    return this->getRGBImgLoss(image, frameid);
}

void GlobalModel::renderImage(const Eigen::Matrix4f &view)
{
    pangolin::GlFramebuffer imageFramebuffer;
    imageFramebuffer.AttachColour(imageTexture);
    imageFramebuffer.AttachColour(semanticTexture);
    //imageFramebuffer.AttachColour(depthTexture);
    imageFramebuffer.AttachDepth(imageRenderBuffer);

    imageFramebuffer.Bind();

    glPushAttrib(GL_VIEWPORT_BIT);

    glViewport(0, 0, imageRenderBuffer.width, imageRenderBuffer.height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawImageProgram->Bind();

    Eigen::Matrix4f t_inv = view.inverse();
    drawImageProgram->setUniform(Uniform("t_inv", t_inv));
    drawImageProgram->setUniform(Uniform("cam", imageCam));
    drawImageProgram->setUniform(Uniform("cols", (float)imageRenderBuffer.width));
    drawImageProgram->setUniform(Uniform("rows", (float)imageRenderBuffer.height));
    drawImageProgram->setUniform(Uniform("maxDepth", 200.f));
    drawImageProgram->setUniform(Uniform("threshold", 0.f));

    glBindBuffer(GL_ARRAY_BUFFER, modelVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 1));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Config::vertexSize(), reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    glDrawArrays(GL_POINTS, 0, count);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    drawImageProgram->Unbind();

    glPopAttrib();

    imageFramebuffer.Unbind();

    glFinish();

    CheckGlDieOnError()
}

pangolin::GlTexture * GlobalModel::getModelMapVC()
{
    return modelMapVertsConfs.texture;
}

pangolin::GlTexture * GlobalModel::getModelMapCT()
{
    return modelMapColorsTime.texture;
}

pangolin::GlTexture * GlobalModel::getModelMapNR()
{
    return modelMapNormsRadii.texture;
}

pangolin::GlTexture * GlobalModel::getImageTex()
{
    return &imageTexture;
}

pangolin::GlTexture * GlobalModel::getSemanticTex()
{
    return &semanticTexture;
}

std::pair<GLuint, GLuint> GlobalModel::getModel()
{
    return {modelVbo, count};
}

std::pair<GLuint, GLuint> GlobalModel::getRenderBuffer()
{
    return {renderVbo, renderCount};
}

std::pair<GLuint, GLuint> GlobalModel::getmpBuffer()
{
    return {tmpVbo, tmpCount};
}

std::pair<GLuint, GLuint> GlobalModel::getLocalModel()
{
    return {lsmVbo, lsmcount};
}

std::pair<GLuint, GLuint> GlobalModel::getGSMView()
{
    return {gsmViewVbo, gsmViewcount};
}

std::pair<GLuint, GLuint> GlobalModel::getData()
{
    return {dataVbo, dataCount};
}

std::pair<GLuint, GLuint> GlobalModel::getConflict()
{
    return {conflictVbo, conflictCount};
}

std::pair<GLuint, GLuint> GlobalModel::getUnstable()
{
    return {unstableVbo, unstableCount};
}

unsigned int GlobalModel::getOffset()
{
    return offset;
}

void GlobalModel::clearBuffer(GLuint buffer, GLfloat value)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glClearBufferData(GL_ARRAY_BUFFER, GL_R32F, GL_RED, GL_FLOAT, &value);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CheckGlDieOnError();
}

bool GlobalModel::downloadMap(const std::string &path, int startId, int endId)
{
    glFinish();

    float * vertices = new float[count * sizeof(float) * 12];

    memset(vertices, 0, count * sizeof(float) * 12);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, modelVbo);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, count * sizeof(float) * 12, vertices);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

    glFinish();

    CheckGlDieOnError()

    std::ofstream file(path, std::ios::binary);

    unsigned int vert_num = count;
    int start_id = startId;
    int end_id = endId;

    try
    {
        if(file.is_open())
        {
            file.write((char *)&vert_num, sizeof(unsigned int));
            file.write((char *)&start_id, sizeof(int));
            file.write((char *)&end_id, sizeof(int));

            assert(file.tellp() == sizeof(unsigned int) + 2 * sizeof(int));
            file.write((char *)vertices, count * sizeof(float) * 12);

            file.close();

            printf("%s is saved! Saved model count: %d\n", path.c_str(), count);
        }
        else
        {
            printf("%s is not open!\n", path.c_str());
            return false;
        }
    }
    catch (std::ofstream::failure &e)
    {
        std::cerr << path << " saved err!!" << " " << e.what() << std::endl;
        return false;
    }

    delete [] vertices;

    return true;
}

bool GlobalModel::uploadMap(const std::string &model_path, std::vector<int> &start_end_ids)
{
    unsigned int vert_num;
    float * vertices;
    int start_id;
    int end_id;

    std::ifstream file(model_path, std::ios::binary);

    try
    {
        if(file.is_open())
        {
            file.read((char *)&vert_num, sizeof(unsigned int));

            printf("Load model count: %d\n", vert_num);

            file.read((char *)&start_id, sizeof(int));
            file.read((char *)&end_id, sizeof(int));

            vertices = new float[vert_num * sizeof(float) * 12];

            assert(file.tellg() == sizeof(unsigned int) + 2 * sizeof(int));
            file.read((char *)vertices, vert_num * sizeof(float) * 12);
            file.close();

            printf("Read model from %s.\n", model_path.c_str());
        }
        else
        {
            printf("%s is not open!\n", model_path.c_str());
            return false;
        }
    }
    catch (std::ifstream::failure &e)
    {
        std::cerr << model_path << " read err!!" << " " << e.what() << std::endl;
        return false;
    }

    count = vert_num;
    start_end_ids.clear();
    start_end_ids.push_back(start_id);
    start_end_ids.push_back(end_id);

    glBindBuffer(GL_ARRAY_BUFFER, modelVbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(float) * 12, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glFinish();

    CheckGlDieOnError()

    delete [] vertices;

    return true;
}

void GlobalModel::getSurfelModelData(){
    float * vertices = new float[count * sizeof(float) * 12];

    memset(vertices, 0, count * sizeof(float) * 12);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, modelVbo);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, count * sizeof(float) * 12, vertices);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

//    glFinish();

    CheckGlDieOnError()

    // vertex Position 4 float          vertex Color 4 float            vertex NormRad 4 float
    // Position x -> x-axis
    // Position y -> y-axis
    // Position z -> depth
    // Position w -> confidence
    // Color x -> color (encoded)
    // Color y -> marks as the ID of model surfel to be updated
    // Color z -> old time
    // Color w -> time / tick
    // NormRad x -> x-axis
    // NormRad y -> y-axis
    // NormRad z -> depth/z-axis
    // NormRad w -> radius

    delete [] vertices;
}

float * GlobalModel::getVertexDataFromBuffer(GLuint buffer, int vcount){
    // 12 means 12 float, 4-4-4
    float * vertices = new float[vcount * sizeof(float) * 12];

    memset(vertices, 0, vcount * sizeof(float) * 12);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffer);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vcount * sizeof(float) * 12, vertices);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

    glFinish();

    CheckGlDieOnError()

    // vertex Position 4 float          vertex Color 4 float            vertex NormRad 4 float
    // Position x -> x-axis
    // Position y -> y-axis
    // Position z -> depth
    // Position w -> confidence
    // Color x -> color (encoded)
    // Color y -> marks as the ID of model surfel to be updated
    // Color z -> old time
    // Color w -> time / tick
    // NormRad x -> x-axis
    // NormRad y -> y-axis
    // NormRad z -> depth/z-axis
    // NormRad w -> radius

    // for debug
//    Eigen::MatrixXf vertex;
//    vertex = Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>(vertices, vcount, 12);

//    std::ofstream fout("matrixTest", std::ios::app);
//    fout << vertex << std::endl;
//    fout.flush();

    return vertices;
}

void GlobalModel::getLocalSurfelModel(const Eigen::Matrix4f & pose,
                                      const int & time,
                                      GPUTexture * rgb,
                                      GPUTexture * depthRaw,
                                      GPUTexture * semantic,
                                      GPUTexture * indexMap,
                                      GPUTexture * vertConfMap,
                                      GPUTexture * colorTimeMap,
                                      GPUTexture * normRadMap,
                                      float depthMin,
                                      float depthMax){
    TICK("Data::GetLSM");

    //This part computes new vertices and the vertices to merge with, storing
    //in an array that sets which vertices to update by index
    genLSMProgram->Bind();

    genLSMProgram->setUniform(Uniform("cSampler", 0));
    genLSMProgram->setUniform(Uniform("drSampler", 1));
    genLSMProgram->setUniform(Uniform("sSampler", 2));
    genLSMProgram->setUniform(Uniform("indexSampler", 3));
    genLSMProgram->setUniform(Uniform("vertConfSampler", 4));
    genLSMProgram->setUniform(Uniform("colorTimeSampler", 5));
    genLSMProgram->setUniform(Uniform("normRadSampler", 6));
    genLSMProgram->setUniform(Uniform("time", (float)time));

    genLSMProgram->setUniform(Uniform("cam", Eigen::Vector4f(Config::cx(),
                                                           Config::cy(),
                                                           1.0 / Config::fx(),
                                                           1.0 / Config::fy())));
    genLSMProgram->setUniform(Uniform("cols", (float)Config::W()));
    genLSMProgram->setUniform(Uniform("rows", (float)Config::H()));
    genLSMProgram->setUniform(Uniform("scale", IndexMap::FACTOR));
    genLSMProgram->setUniform(Uniform("texDim", (float)TEXTURE_DIMENSION));
    genLSMProgram->setUniform(Uniform("pose", pose));
    genLSMProgram->setUniform(Uniform("minDepth", depthMin));
    genLSMProgram->setUniform(Uniform("maxDepth", depthMax));
    genLSMProgram->setUniform(Uniform("fuseThresh", Config::surfelFuseDistanceThreshFactor()));

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, uvo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnable(GL_RASTERIZER_DISCARD);
    // use GPU to accerlate the progress
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, lsmFid);
    //glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, dataVbo);
    glTransformFeedbackBufferBase(lsmFid, 0, lsmVbo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rgb->texture->tid);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthRaw->texture->tid);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, semantic->texture->tid);
    // pixel -> Surfel (indexMap)
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, indexMap->texture->tid);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, vertConfMap->texture->tid);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, colorTimeMap->texture->tid);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, normRadMap->texture->tid);

    glBeginTransformFeedback(GL_POINTS);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, countQuery);
    lsmcount = 0;

    glDrawArrays(GL_POINTS, 0, uvSize);

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glGetQueryObjectuiv(countQuery, GL_QUERY_RESULT, &lsmcount);

    glEndTransformFeedback();

    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    glDisable(GL_RASTERIZER_DISCARD);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    genLSMProgram->Unbind();

    glFinish();

    TOCK("Data::GetLSM");

    CheckGlDieOnError()
}

void GlobalModel::getGlobalSurfelInView(const Eigen::Matrix4f & pose,
                                        GPUTexture * indexMap,
                                        GPUTexture * vertConfMap,
                                        GPUTexture * colorTimeMap,
                                        GPUTexture * normRadMap){
    TICK("Data::GetGlobalView");

    //This part computes new vertices and the vertices to merge with, storing
    //in an array that sets which vertices to update by index
    getGSMViewProgram->Bind();

    getGSMViewProgram->setUniform(Uniform("indexSampler", 0));
    getGSMViewProgram->setUniform(Uniform("vertConfSampler", 1));
    getGSMViewProgram->setUniform(Uniform("colorTimeSampler", 2));
    getGSMViewProgram->setUniform(Uniform("normRadSampler", 3));

    getGSMViewProgram->setUniform(Uniform("pose", pose));


    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, uvo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnable(GL_RASTERIZER_DISCARD);
    // use GPU to accerlate the progress
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, gsmViewFid);
    //glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, dataVbo);
    glTransformFeedbackBufferBase(gsmViewFid, 0, gsmViewVbo);

    // pixel -> Surfel (indexMap)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, indexMap->texture->tid);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, vertConfMap->texture->tid);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, colorTimeMap->texture->tid);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, normRadMap->texture->tid);

    glBeginTransformFeedback(GL_POINTS);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, countQuery);
    gsmViewcount = 0;

    glDrawArrays(GL_POINTS, 0, uvSize);

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    glGetQueryObjectuiv(countQuery, GL_QUERY_RESULT, &gsmViewcount);

    glEndTransformFeedback();

    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    glDisable(GL_RASTERIZER_DISCARD);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    getGSMViewProgram->Unbind();

    glFinish();

    TOCK("Data::GetGlobalView");

    CheckGlDieOnError()
}

void GlobalModel::ICP(GLuint GlobalSurfelInView, int ViewCount, GLuint LSModel, int LSMcount) {

    // vertex Position 4 float          vertex Color 4 float            vertex NormRad 4 float
    // Position x -> x-axis
    // Position y -> y-axis
    // Position z -> depth
    // Position w -> confidence
    // Color x -> color (encoded)
    // Color y -> marks as the ID of model surfel to be updated
    // Color z -> old time
    // Color w -> time / tick
    // NormRad x -> x-axis
    // NormRad y -> y-axis
    // NormRad z -> depth/z-axis
    // NormRad w -> radius

    float * GSMView = this->getVertexDataFromBuffer(GlobalSurfelInView, ViewCount);
    float * LSM = this->getVertexDataFromBuffer(LSModel, LSMcount);
    Eigen::MatrixXf GSMvertex = Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>(GSMView, ViewCount, 12);
    Eigen::MatrixXf LSMvertex = Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>(LSM, LSMcount, 12);
    
}