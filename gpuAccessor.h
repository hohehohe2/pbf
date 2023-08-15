#ifndef HOHE2_gpuAccessor_H
#define HOHE2_gpuAccessor_H
#include <OpenCL/opencl.h>

namespace hohe2
{

///Class to hold logics for GPU.
class GpuAccessor
{
    cl_platform_id platform_id;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue command_queue;
    cl_program program;

    cl_kernel kernelDensity;
    cl_kernel kernelPressure;
    cl_kernel kernelViscosityFluid;
    cl_kernel kernelViscosityWall;
    cl_kernel kernelTest;

    cl_mem bufferSelfPos;
    cl_mem bufferSelfVeloc;
    cl_mem bufferSelfDens;
    cl_mem bufferNeighborIndex;
    cl_mem bufferNumNeighbors;
    cl_mem bufferOutData;
    cl_mem imagePosDens;
    cl_mem imageVeloc;

    //----

    size_t numBlocks_;
    size_t numThreads_;
    size_t localItemSize;

    int numIterations_;

    float* selfPosArray_;
    float* selfVelocArray_;
    float* selfDensArray_;

    int* neighborIndexArray_;
    int* numNeighborsArray_;
    float* outNeighborPosDensArray_;
    float* outNeighborVelocArray_;

    float* outDataArray_;

    int numParticleDataSize;
    int numImageDataSize;
    int imageWidth_;
    int imageHeight_;

    size_t origin[3];
    size_t region[3];

public:
    bool initlalize(const char* fileName,
                    int numBlocks,
                    int numThreads,
                    int numIterations);
    void setSelfArrays(float* selfPosArray, float* selfVelocArray, float* selfDensArray);
    void getBuffers(int*& neighborIndexArray, int*& numNeighborsArray,
                    float*& outNeighborPosDensArray, float*& outNeighborVelocArray,
                    float*& outDataArray);
    bool runDensity(int numParticles);
    bool runPressure(int numParticles);
    bool runViscosityFluid(int numParticles);
    bool runViscosityWall(int numParticles);
    bool runTest();

    void cleanUp();

private:
    bool createKernel_(const char* fileName);
};

}

#endif
