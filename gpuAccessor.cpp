#include "gpuAccessor.h"
#include <math.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include "sph.h"

using namespace hohe2;

static void checkCreateImage2D(cl_int ret);
static void checkCreateKernel(cl_int ret);
static void checkSetKernelArg(cl_int ret);
static void checkEnqueueNDRangeKernel(cl_int ret);
static void checkRWImage(cl_int ret);


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool GpuAccessor::initlalize(const char* fileName, int numBlocks, int numThreads, int numIterations)
{
    numBlocks_ = numBlocks;
    numThreads_ = numThreads;
    numIterations_ = numIterations;

    localItemSize = numThreads_ / numBlocks_;
    numParticleDataSize = numIterations_ * numThreads_ * Sph::NUM_EVERY_NEIGHBOR_CELLS;
    numImageDataSize = numIterations_ * numThreads_ * Sph::NUM_EVERY_NEIGHBOR_CELLS * 8 * Sph::CHANNELDEPTH; //8 is the number of particles when the fluid is in a "rest" state. TODO: May not be enough. How can we know the enough size?

    unsigned width = 1;
    while(true)
    {
        if (width * width > numIterations_ * numThreads_ * Sph::NUM_EVERY_NEIGHBOR_CELLS * 8)
        {
            break;
        }
        width *= 2;
    }
    imageWidth_ = width;
    imageHeight_ = width;

    origin[0] = origin[1] = origin[2] = 0;
    region[0] = imageWidth_;
    region[1] = imageHeight_;
    region[2] = 1;



    neighborIndexArray_ = new int[numParticleDataSize];
    numNeighborsArray_ = new int[numParticleDataSize];
    outNeighborPosDensArray_ = new float[numImageDataSize];
    outNeighborVelocArray_ = new float[numImageDataSize];
    outDataArray_ = new float[numIterations_ * numThreads_ * 3];

    cl_int ret;

    cl_uint ret_num_platforms;
    clGetPlatformIDs(1, &platform_id, &ret_num_platforms);

    cl_uint ret_num_devices;
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

    context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);

    /* イメージのサポートのチェック */
    size_t r_size;
    cl_bool support;
    clGetDeviceInfo(device_id, CL_DEVICE_IMAGE_SUPPORT, sizeof(support), &support, &r_size);
    if (support != CL_TRUE) {
        puts("image not supported");
        return false;
    }

    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    if (ret != CL_SUCCESS)
    {
        std::cerr << "err clCreateCommandQueue\n";
        exit(1);
    }

    //Create buffer objects for neighborIndexArray, numNeighborsArray, and outDataArray
    bufferNeighborIndex = clCreateBuffer(context, CL_MEM_READ_ONLY, numParticleDataSize * sizeof(int), NULL, &ret);
    bufferNumNeighbors = clCreateBuffer(context, CL_MEM_READ_ONLY, numParticleDataSize * sizeof(int), NULL, &ret);
    bufferSelfPos = clCreateBuffer(context, CL_MEM_READ_ONLY, numParticleDataSize * 3 * sizeof(float), NULL, &ret);
    bufferSelfVeloc = clCreateBuffer(context, CL_MEM_READ_ONLY, numParticleDataSize * 3 * sizeof(float), NULL, &ret);
    bufferSelfDens = clCreateBuffer(context, CL_MEM_READ_ONLY, numParticleDataSize * 1 * sizeof(float), NULL, &ret);
    bufferOutData = clCreateBuffer(context, CL_MEM_WRITE_ONLY, numIterations_ * numThreads_ * 3 * sizeof(float), NULL, &ret);

    //Create image objects for outNeighborPosDens/Veloc.
    cl_image_format fmt;
    fmt.image_channel_order = CL_RGBA;
    fmt.image_channel_data_type = CL_FLOAT;
    imagePosDens = clCreateImage2D(context, CL_MEM_READ_ONLY, &fmt, imageWidth_, imageHeight_, 0, NULL, &ret);
    checkCreateImage2D(ret);
    imageVeloc = clCreateImage2D(context, CL_MEM_READ_ONLY, &fmt, imageWidth_, imageHeight_, 0, NULL, &ret);
    checkCreateImage2D(ret);

    if (createKernel_(fileName))
    {
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void GpuAccessor::setSelfArrays(float* selfPosArray, float* selfVelocArray, float* selfDensArray)
{
    selfPosArray_ = selfPosArray;
    selfVelocArray_ = selfVelocArray;
    selfDensArray_ = selfDensArray;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void GpuAccessor::getBuffers(int*& neighborIndexArray, int*& numNeighborsArray,
                                  float*& outNeighborPosDensArray, float*& outNeighborVelocArray,
                                  float*& outDataArray)
{
    neighborIndexArray = neighborIndexArray_;
    numNeighborsArray = numNeighborsArray_;
    outNeighborPosDensArray = outNeighborPosDensArray_;
    outNeighborVelocArray = outNeighborVelocArray_;
    outDataArray = outDataArray_;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool GpuAccessor::runDensity(int numParticles)
{
    cl_int ret = clSetKernelArg(kernelTest, 0, sizeof(int), &numParticles);
    checkSetKernelArg(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferNeighborIndex, CL_TRUE, 0, numParticleDataSize * sizeof(int), neighborIndexArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferNumNeighbors, CL_TRUE, 0, numParticleDataSize * sizeof(int), numNeighborsArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferSelfPos, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), selfPosArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteImage(command_queue, imagePosDens, CL_TRUE, origin, region, 0, 0, outNeighborPosDensArray_, 0, NULL, NULL);
    checkRWImage(ret);

    clEnqueueReadBuffer(command_queue, bufferOutData, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), outDataArray_, 0, NULL, NULL);
    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool GpuAccessor::runPressure(int numParticles)
{
    cl_int ret = clSetKernelArg(kernelTest, 0, sizeof(int), &numParticles);
    checkSetKernelArg(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferNeighborIndex, CL_TRUE, 0, numParticleDataSize * sizeof(int), neighborIndexArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferNumNeighbors, CL_TRUE, 0, numParticleDataSize * sizeof(int), numNeighborsArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferSelfPos, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), selfPosArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferSelfDens, CL_TRUE, 0, numIterations_ * numThreads_ * 1 * sizeof(float), selfDensArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteImage(command_queue, imagePosDens, CL_TRUE, origin, region, 0, 0, outNeighborPosDensArray_, 0, NULL, NULL);
    checkRWImage(ret);

    clEnqueueReadBuffer(command_queue, bufferOutData, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), outDataArray_, 0, NULL, NULL);
    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool GpuAccessor::runViscosityFluid(int numParticles)
{
    cl_int ret = clSetKernelArg(kernelTest, 0, sizeof(int), &numParticles);
    checkSetKernelArg(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferNeighborIndex, CL_TRUE, 0, numParticleDataSize * sizeof(int), neighborIndexArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferNumNeighbors, CL_TRUE, 0, numParticleDataSize * sizeof(int), numNeighborsArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferSelfPos, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), selfPosArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferSelfVeloc, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), selfVelocArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteImage(command_queue, imagePosDens, CL_TRUE, origin, region, 0, 0, outNeighborPosDensArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteImage(command_queue, imageVeloc, CL_TRUE, origin, region, 0, 0, outNeighborVelocArray_, 0, NULL, NULL);
    checkRWImage(ret);

    clEnqueueReadBuffer(command_queue, bufferOutData, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), outDataArray_, 0, NULL, NULL);
    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool GpuAccessor::runViscosityWall(int numParticles)
{
    cl_int ret = clSetKernelArg(kernelTest, 0, sizeof(int), &numParticles);
    checkSetKernelArg(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferNeighborIndex, CL_TRUE, 0, numParticleDataSize * sizeof(int), neighborIndexArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferNumNeighbors, CL_TRUE, 0, numParticleDataSize * sizeof(int), numNeighborsArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferSelfPos, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), selfPosArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferSelfVeloc, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), selfVelocArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteImage(command_queue, imagePosDens, CL_TRUE, origin, region, 0, 0, outNeighborPosDensArray_, 0, NULL, NULL);
    checkRWImage(ret);

    clEnqueueReadBuffer(command_queue, bufferOutData, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), outDataArray_, 0, NULL, NULL);
    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool GpuAccessor::runTest()
{
    cl_int ret;

    int numParticles = 99;
    ret = clSetKernelArg(kernelTest, 0, sizeof(int), &numParticles);
    checkSetKernelArg(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferNeighborIndex, CL_TRUE, 0, numParticleDataSize * sizeof(int), neighborIndexArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferNumNeighbors, CL_TRUE, 0, numParticleDataSize * sizeof(int), numNeighborsArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferSelfPos, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), selfPosArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferSelfVeloc, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), selfVelocArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteBuffer(command_queue, bufferSelfDens, CL_TRUE, 0, numIterations_ * numThreads_ * 1 * sizeof(float), selfDensArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteImage(command_queue, imagePosDens, CL_TRUE, origin, region, 0, 0, outNeighborPosDensArray_, 0, NULL, NULL);
    checkRWImage(ret);
    ret = clEnqueueWriteImage(command_queue, imageVeloc, CL_TRUE, origin, region, 0, 0, outNeighborVelocArray_, 0, NULL, NULL);
    checkRWImage(ret);

    ret = clEnqueueNDRangeKernel(command_queue, kernelTest, 1, NULL, &numThreads_, &localItemSize, 0, NULL, NULL);
    checkEnqueueNDRangeKernel(ret);

    clEnqueueReadBuffer(command_queue, bufferOutData, CL_TRUE, 0, numIterations_ * numThreads_ * 3 * sizeof(float), outDataArray_, 0, NULL, NULL);

    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void GpuAccessor::cleanUp()
{
    clReleaseMemObject(bufferSelfPos);
    clReleaseMemObject(bufferSelfVeloc);
    clReleaseMemObject(bufferSelfDens);
    clReleaseMemObject(bufferNeighborIndex);
    clReleaseMemObject(bufferNumNeighbors);
    clReleaseMemObject(bufferOutData);
    clReleaseMemObject(imagePosDens);
    clReleaseMemObject(imageVeloc);

    clReleaseKernel(kernelDensity);
    clReleaseKernel(kernelPressure);
    clReleaseKernel(kernelViscosityFluid);
    clReleaseKernel(kernelViscosityWall);
    clReleaseKernel(kernelTest);

    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool GpuAccessor::createKernel_(const char* fileName)
{
    cl_int ret;

    static const int MAX_SOURCE_SIZE = 0x100000;

    FILE* fp = fopen(fileName, "r");
    char* kernel_src_str = (char*)malloc(MAX_SOURCE_SIZE);
    size_t kernel_code_size = fread(kernel_src_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    program = clCreateProgramWithSource(context, 1, (const char**)&kernel_src_str, &kernel_code_size, &ret);
    if (ret)
    {
        std::cerr << "err create progam: " << fileName << std::endl;
        exit(1);
    }
    free(kernel_src_str);

    ret = clBuildProgram(program, 1, &device_id, "", NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        std::cerr << "err build program: " << fileName << std::endl;
        exit(1);
    }

    kernelDensity = clCreateKernel(program, "density", &ret);
    checkCreateKernel(ret);

    kernelPressure = clCreateKernel(program, "pressure", &ret);
    checkCreateKernel(ret);

    kernelViscosityFluid = clCreateKernel(program, "viscosityFluid", &ret);
    checkCreateKernel(ret);

    kernelViscosityWall = clCreateKernel(program, "viscosityWall", &ret);
    checkCreateKernel(ret);

    kernelTest = clCreateKernel(program, "test", &ret);
    checkCreateKernel(ret);
    ret = clSetKernelArg(kernelTest, 1, sizeof(cl_mem), &bufferNeighborIndex);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelTest, 2, sizeof(cl_mem), &bufferNumNeighbors);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelTest, 3, sizeof(cl_mem), &bufferSelfPos);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelTest, 4, sizeof(cl_mem), &bufferSelfVeloc);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelTest, 5, sizeof(cl_mem), &bufferSelfDens);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelTest, 6, sizeof(cl_mem), &imagePosDens);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelTest, 7, sizeof(cl_mem), &imageVeloc);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelTest, 8, sizeof(cl_mem), &bufferOutData);
    checkSetKernelArg(ret);

    kernelDensity = clCreateKernel(program, "density", &ret);
    checkCreateKernel(ret);
    ret = clSetKernelArg(kernelDensity, 1, sizeof(cl_mem), &bufferNeighborIndex);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelDensity, 2, sizeof(cl_mem), &bufferNumNeighbors);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelDensity, 3, sizeof(cl_mem), &bufferSelfPos);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelDensity, 4, sizeof(cl_mem), &imagePosDens);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelDensity, 5, sizeof(cl_mem), &bufferOutData);
    checkSetKernelArg(ret);

    kernelPressure = clCreateKernel(program, "pressure", &ret);
    checkCreateKernel(ret);
    ret = clSetKernelArg(kernelPressure, 1, sizeof(cl_mem), &bufferNeighborIndex);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelPressure, 2, sizeof(cl_mem), &bufferNumNeighbors);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelPressure, 3, sizeof(cl_mem), &bufferSelfPos);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelPressure, 4, sizeof(cl_mem), &bufferSelfDens);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelPressure, 5, sizeof(cl_mem), &imagePosDens);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelPressure, 6, sizeof(cl_mem), &bufferOutData);
    checkSetKernelArg(ret);

    kernelViscosityFluid = clCreateKernel(program, "viscosityFluid", &ret);
    checkCreateKernel(ret);
    ret = clSetKernelArg(kernelViscosityFluid, 1, sizeof(cl_mem), &bufferNeighborIndex);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelViscosityFluid, 2, sizeof(cl_mem), &bufferNumNeighbors);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelViscosityFluid, 3, sizeof(cl_mem), &bufferSelfPos);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelViscosityFluid, 4, sizeof(cl_mem), &bufferSelfVeloc);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelViscosityFluid, 5, sizeof(cl_mem), &imagePosDens);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelViscosityFluid, 6, sizeof(cl_mem), &imageVeloc);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelViscosityFluid, 7, sizeof(cl_mem), &bufferOutData);
    checkSetKernelArg(ret);

    kernelViscosityWall = clCreateKernel(program, "viscosityWall", &ret);
    checkCreateKernel(ret);
    ret = clSetKernelArg(kernelViscosityWall, 1, sizeof(cl_mem), &bufferNeighborIndex);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelViscosityWall, 2, sizeof(cl_mem), &bufferNumNeighbors);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelViscosityWall, 3, sizeof(cl_mem), &bufferSelfPos);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelViscosityWall, 4, sizeof(cl_mem), &bufferSelfVeloc);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelViscosityWall, 5, sizeof(cl_mem), &imagePosDens);
    checkSetKernelArg(ret);
    ret = clSetKernelArg(kernelViscosityWall, 6, sizeof(cl_mem), &bufferOutData);
    checkSetKernelArg(ret);


    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void checkCreateImage2D(cl_int ret)
{
    if (ret != CL_SUCCESS)
    {
        std::cerr << "err clCreateImage2D\n";
        if (ret == CL_INVALID_CONTEXT)
        {
            std::cerr << "CL_INVALID_CONTEXT";
        }
        else if (ret == CL_INVALID_VALUE)
        {
            std::cerr << "CL_INVALID_VALUE";
        }
        else if (ret == CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
        {
            std::cerr << "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        }
        else if (ret == CL_INVALID_IMAGE_SIZE)
        {
            std::cerr << "CL_INVALID_IMAGE_SIZE";
        }
        else if (ret == CL_INVALID_HOST_PTR)
        {
            std::cerr << "CL_INVALID_HOST_PTR";
        }
        else if (ret == CL_IMAGE_FORMAT_NOT_SUPPORTED)
        {
            std::cerr << "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        }
        else if (ret == CL_MEM_OBJECT_ALLOCATION_FAILURE)
        {
            std::cerr << "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        }
        else if (ret == CL_INVALID_OPERATION)
        {
            std::cerr << "CL_INVALID_OPERATION";
        }
        else if (ret == CL_OUT_OF_HOST_MEMORY)
        {
            std::cerr << "CL_OUT_OF_HOST_MEMORY";
        }
        exit(1);
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void checkCreateKernel(cl_int ret)
{
    if (ret != CL_SUCCESS)
    {
        std::cerr << "err checkCreateKernel " << ret << std::endl;
        std::cerr <<
            CL_INVALID_KERNEL_NAME << " " << 
            CL_INVALID_KERNEL_DEFINITION << " " << 
            CL_INVALID_VALUE << " " << 
            CL_OUT_OF_HOST_MEMORY << std::endl;
        exit(1);
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void checkSetKernelArg(cl_int ret)
{
    if (ret != CL_SUCCESS)
    {
        std::cerr << "err checkSetKernelArg " << ret << std::endl;
        std::cerr <<
            CL_INVALID_KERNEL << " " << 
            CL_INVALID_ARG_INDEX << " " << 
            CL_INVALID_ARG_VALUE << " " << 
            CL_INVALID_MEM_OBJECT << " " << 
            CL_INVALID_SAMPLER << " " << 
            CL_INVALID_ARG_SIZE << std::endl;
        exit(1);
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void checkEnqueueNDRangeKernel(cl_int ret)
{
    if (ret != CL_SUCCESS)
    {
        std::cerr << "err checkEnqueueNDRangeKernel " << ret << std::endl;
        std::cerr <<
            CL_INVALID_PROGRAM_EXECUTABLE << " " << 
            CL_INVALID_COMMAND_QUEUE << " " << 
            CL_INVALID_KERNEL << " " << 
            CL_INVALID_CONTEXT << " " << 
            CL_INVALID_KERNEL_ARGS << " " << 
            CL_INVALID_WORK_DIMENSION << " " << 
            CL_INVALID_GLOBAL_WORK_SIZE << " " << 
            CL_INVALID_WORK_GROUP_SIZE << " " << 
            CL_INVALID_WORK_GROUP_SIZE << " " << 
            CL_INVALID_WORK_GROUP_SIZE << " " << 
            CL_INVALID_WORK_ITEM_SIZE << " " << 
            CL_INVALID_GLOBAL_OFFSET << " " << 
            CL_OUT_OF_RESOURCES << " " << 
            CL_MEM_OBJECT_ALLOCATION_FAILURE << " " << 
            CL_INVALID_EVENT_WAIT_LIST << " " << 
            CL_OUT_OF_HOST_MEMORY << std::endl;
        exit(1);
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void checkRWImage(cl_int ret)
{
    if (ret != CL_SUCCESS)
    {
        std::cerr << "err checkRWImage " << ret << std::endl;
        std::cerr <<
            CL_INVALID_COMMAND_QUEUE << " " << 
            CL_INVALID_CONTEXT << " " << 
            CL_INVALID_MEM_OBJECT << " " << 
            CL_INVALID_VALUE << " " << 
            CL_INVALID_EVENT_WAIT_LIST << " " << 
            CL_MEM_OBJECT_ALLOCATION_FAILURE << " " << 
            CL_INVALID_OPERATION << " " << 
            CL_OUT_OF_HOST_MEMORY << std::endl;
        exit(1);
    }
}
