/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef __OPENCV_CUDAOPTFLOW_HPP__
#define __OPENCV_CUDAOPTFLOW_HPP__

#ifndef __cplusplus
#  error cudaoptflow.hpp header must be compiled as C++
#endif

#include "opencv2/core/cuda.hpp"

/**
  @addtogroup cuda
  @{
    @defgroup cudaoptflow Optical Flow
  @}
 */

namespace cv { namespace cuda {

//! @addtogroup cudaoptflow
//! @{

/** @brief Class computing the optical flow for two images using Brox et al Optical Flow algorithm
(@cite Brox2004). :
 */
class CV_EXPORTS BroxOpticalFlow
{
public:
    BroxOpticalFlow(float alpha_, float gamma_, float scale_factor_, int inner_iterations_, int outer_iterations_, int solver_iterations_) :
        alpha(alpha_), gamma(gamma_), scale_factor(scale_factor_),
        inner_iterations(inner_iterations_), outer_iterations(outer_iterations_), solver_iterations(solver_iterations_)
    {
    }

    //! Compute optical flow
    //! frame0 - source frame (supports only CV_32FC1 type)
    //! frame1 - frame to track (with the same size and type as frame0)
    //! u      - flow horizontal component (along x axis)
    //! v      - flow vertical component (along y axis)
    void operator ()(const GpuMat& frame0, const GpuMat& frame1, GpuMat& u, GpuMat& v, Stream& stream = Stream::Null());

    //! flow smoothness
    float alpha;

    //! gradient constancy importance
    float gamma;

    //! pyramid scale factor
    float scale_factor;

    //! number of lagged non-linearity iterations (inner loop)
    int inner_iterations;

    //! number of warping iterations (number of pyramid levels)
    int outer_iterations;

    //! number of linear system solver iterations
    int solver_iterations;

    GpuMat buf;
};

/** @brief Class used for calculating an optical flow.

The class can calculate an optical flow for a sparse feature set or dense optical flow using the
iterative Lucas-Kanade method with pyramids.

@sa calcOpticalFlowPyrLK

@note
   -   An example of the Lucas Kanade optical flow algorithm can be found at
        opencv_source_code/samples/gpu/pyrlk_optical_flow.cpp
 */
class CV_EXPORTS PyrLKOpticalFlow
{
public:
    PyrLKOpticalFlow();

    /** @brief Calculate an optical flow for a sparse feature set.

    @param prevImg First 8-bit input image (supports both grayscale and color images).
    @param nextImg Second input image of the same size and the same type as prevImg .
    @param prevPts Vector of 2D points for which the flow needs to be found. It must be one row matrix
    with CV_32FC2 type.
    @param nextPts Output vector of 2D points (with single-precision floating-point coordinates)
    containing the calculated new positions of input features in the second image. When useInitialFlow
    is true, the vector must have the same size as in the input.
    @param status Output status vector (CV_8UC1 type). Each element of the vector is set to 1 if the
    flow for the corresponding features has been found. Otherwise, it is set to 0.
    @param err Output vector (CV_32FC1 type) that contains the difference between patches around the
    original and moved points or min eigen value if getMinEigenVals is checked. It can be NULL, if not
    needed.

    @sa calcOpticalFlowPyrLK
     */
    void sparse(const GpuMat& prevImg, const GpuMat& nextImg, const GpuMat& prevPts, GpuMat& nextPts,
        GpuMat& status, GpuMat* err = 0);

    /** @brief Calculate dense optical flow.

    @param prevImg First 8-bit grayscale input image.
    @param nextImg Second input image of the same size and the same type as prevImg .
    @param u Horizontal component of the optical flow of the same size as input images, 32-bit
    floating-point, single-channel
    @param v Vertical component of the optical flow of the same size as input images, 32-bit
    floating-point, single-channel
    @param err Output vector (CV_32FC1 type) that contains the difference between patches around the
    original and moved points or min eigen value if getMinEigenVals is checked. It can be NULL, if not
    needed.
     */
    void dense(const GpuMat& prevImg, const GpuMat& nextImg, GpuMat& u, GpuMat& v, GpuMat* err = 0);

    /** @brief Releases inner buffers memory.
    */
    void releaseMemory();

    Size winSize;
    int maxLevel;
    int iters;
    bool useInitialFlow;

private:
    std::vector<GpuMat> prevPyr_;
    std::vector<GpuMat> nextPyr_;

    GpuMat buf_;

    GpuMat uPyr_[2];
    GpuMat vPyr_[2];
};

/** @brief Class computing a dense optical flow using the Gunnar Farneback’s algorithm. :
 */
class CV_EXPORTS FarnebackOpticalFlow
{
public:
    FarnebackOpticalFlow()
    {
        numLevels = 5;
        pyrScale = 0.5;
        fastPyramids = false;
        winSize = 13;
        numIters = 10;
        polyN = 5;
        polySigma = 1.1;
        flags = 0;
    }

    int numLevels;
    double pyrScale;
    bool fastPyramids;
    int winSize;
    int numIters;
    int polyN;
    double polySigma;
    int flags;

    /** @brief Computes a dense optical flow using the Gunnar Farneback’s algorithm.

    @param frame0 First 8-bit gray-scale input image
    @param frame1 Second 8-bit gray-scale input image
    @param flowx Flow horizontal component
    @param flowy Flow vertical component
    @param s Stream

    @sa calcOpticalFlowFarneback
     */
    void operator ()(const GpuMat &frame0, const GpuMat &frame1, GpuMat &flowx, GpuMat &flowy, Stream &s = Stream::Null());

    /** @brief Releases unused auxiliary memory buffers.
     */
    void releaseMemory()
    {
        frames_[0].release();
        frames_[1].release();
        pyrLevel_[0].release();
        pyrLevel_[1].release();
        M_.release();
        bufM_.release();
        R_[0].release();
        R_[1].release();
        blurredFrame_[0].release();
        blurredFrame_[1].release();
        pyramid0_.clear();
        pyramid1_.clear();
    }

private:
    void prepareGaussian(
            int n, double sigma, float *g, float *xg, float *xxg,
            double &ig11, double &ig03, double &ig33, double &ig55);

    void setPolynomialExpansionConsts(int n, double sigma);

    void updateFlow_boxFilter(
            const GpuMat& R0, const GpuMat& R1, GpuMat& flowx, GpuMat &flowy,
            GpuMat& M, GpuMat &bufM, int blockSize, bool updateMatrices, Stream streams[]);

    void updateFlow_gaussianBlur(
            const GpuMat& R0, const GpuMat& R1, GpuMat& flowx, GpuMat& flowy,
            GpuMat& M, GpuMat &bufM, int blockSize, bool updateMatrices, Stream streams[]);

    GpuMat frames_[2];
    GpuMat pyrLevel_[2], M_, bufM_, R_[2], blurredFrame_[2];
    std::vector<GpuMat> pyramid0_, pyramid1_;
};

// Implementation of the Zach, Pock and Bischof Dual TV-L1 Optical Flow method
//
// see reference:
//   [1] C. Zach, T. Pock and H. Bischof, "A Duality Based Approach for Realtime TV-L1 Optical Flow".
//   [2] Javier Sanchez, Enric Meinhardt-Llopis and Gabriele Facciolo. "TV-L1 Optical Flow Estimation".
class CV_EXPORTS OpticalFlowDual_TVL1_CUDA
{
public:
    OpticalFlowDual_TVL1_CUDA();

    void operator ()(const GpuMat& I0, const GpuMat& I1, GpuMat& flowx, GpuMat& flowy);

    void collectGarbage();

    /**
     * Time step of the numerical scheme.
     */
    double tau;

    /**
     * Weight parameter for the data term, attachment parameter.
     * This is the most relevant parameter, which determines the smoothness of the output.
     * The smaller this parameter is, the smoother the solutions we obtain.
     * It depends on the range of motions of the images, so its value should be adapted to each image sequence.
     */
    double lambda;

    /**
     * Weight parameter for (u - v)^2, tightness parameter.
     * It serves as a link between the attachment and the regularization terms.
     * In theory, it should have a small value in order to maintain both parts in correspondence.
     * The method is stable for a large range of values of this parameter.
     */

    double gamma;
    /**
    * parameter used for motion estimation. It adds a variable allowing for illumination variations
    * Set this parameter to 1. if you have varying illumination.
    * See: Chambolle et al, A First-Order Primal-Dual Algorithm for Convex Problems with Applications to Imaging
    * Journal of Mathematical imaging and vision, may 2011 Vol 40 issue 1, pp 120-145
    */
    double theta;

    /**
     * Number of scales used to create the pyramid of images.
     */
    int nscales;

    /**
     * Number of warpings per scale.
     * Represents the number of times that I1(x+u0) and grad( I1(x+u0) ) are computed per scale.
     * This is a parameter that assures the stability of the method.
     * It also affects the running time, so it is a compromise between speed and accuracy.
     */
    int warps;

    /**
     * Stopping criterion threshold used in the numerical scheme, which is a trade-off between precision and running time.
     * A small value will yield more accurate solutions at the expense of a slower convergence.
     */
    double epsilon;

    /**
     * Stopping criterion iterations number used in the numerical scheme.
     */
    int iterations;

    double scaleStep;

    bool useInitialFlow;

private:
    void procOneScale(const GpuMat& I0, const GpuMat& I1, GpuMat& u1, GpuMat& u2, GpuMat& u3);

    std::vector<GpuMat> I0s;
    std::vector<GpuMat> I1s;
    std::vector<GpuMat> u1s;
    std::vector<GpuMat> u2s;
    std::vector<GpuMat> u3s;

    GpuMat I1x_buf;
    GpuMat I1y_buf;

    GpuMat I1w_buf;
    GpuMat I1wx_buf;
    GpuMat I1wy_buf;

    GpuMat grad_buf;
    GpuMat rho_c_buf;

    GpuMat p11_buf;
    GpuMat p12_buf;
    GpuMat p21_buf;
    GpuMat p22_buf;
    GpuMat p31_buf;
    GpuMat p32_buf;

    GpuMat diff_buf;
    GpuMat norm_buf;
};

//! Calculates optical flow for 2 images using block matching algorithm */
CV_EXPORTS void calcOpticalFlowBM(const GpuMat& prev, const GpuMat& curr,
                                  Size block_size, Size shift_size, Size max_range, bool use_previous,
                                  GpuMat& velx, GpuMat& vely, GpuMat& buf,
                                  Stream& stream = Stream::Null());

class CV_EXPORTS FastOpticalFlowBM
{
public:
    void operator ()(const GpuMat& I0, const GpuMat& I1, GpuMat& flowx, GpuMat& flowy, int search_window = 21, int block_window = 7, Stream& s = Stream::Null());

private:
    GpuMat buffer;
    GpuMat extended_I0;
    GpuMat extended_I1;
};

/** @brief Interpolates frames (images) using provided optical flow (displacement field).

@param frame0 First frame (32-bit floating point images, single channel).
@param frame1 Second frame. Must have the same type and size as frame0 .
@param fu Forward horizontal displacement.
@param fv Forward vertical displacement.
@param bu Backward horizontal displacement.
@param bv Backward vertical displacement.
@param pos New frame position.
@param newFrame Output image.
@param buf Temporary buffer, will have width x 6\*height size, CV_32FC1 type and contain 6
GpuMat: occlusion masks for first frame, occlusion masks for second, interpolated forward
horizontal flow, interpolated forward vertical flow, interpolated backward horizontal flow,
interpolated backward vertical flow.
@param stream Stream for the asynchronous version.
 */
CV_EXPORTS void interpolateFrames(const GpuMat& frame0, const GpuMat& frame1,
                                  const GpuMat& fu, const GpuMat& fv,
                                  const GpuMat& bu, const GpuMat& bv,
                                  float pos, GpuMat& newFrame, GpuMat& buf,
                                  Stream& stream = Stream::Null());

CV_EXPORTS void createOpticalFlowNeedleMap(const GpuMat& u, const GpuMat& v, GpuMat& vertex, GpuMat& colors);

//! @}

}} // namespace cv { namespace cuda {

#endif /* __OPENCV_CUDAOPTFLOW_HPP__ */
