#include <iostream>
#include <sstream>
#include <CppUnitLite/TestHarness.h>
#include "gpuAccessor.h"

using namespace hohe2;


// TEST(GpuAccessor, create)
// {
//     GpuAccessor ga;
//     ga.initlalize();
//     ga.run();
//     ga.getResult();
//     ga.cleanUp();
//     //DOUBLES_EQUAL(5.0, ps.maxVelicty(), 1.0E-3);
// }

TEST(GpuAccessor, create)
{
    int numBlocks = 1;
    int numThreads = 8;
    int numIterations = 1;

    float selfPosArray[8 * 3];
    float selfVelocArray[8 * 3];
    float selfDensArray[8 * 3];

    GpuAccessor ga;

    ga.initlalize("test.c",
                  numBlocks,
                  numThreads,
                  numIterations);

    int* neighborIndexArray;
    int* numNeighborsArray;
    float* outNeighborPosDensArray;
    float* outNeighborVelocArray;
    float* outDataArray;
    ga.getBuffers(neighborIndexArray, numNeighborsArray,
                  outNeighborPosDensArray, outNeighborVelocArray,
                  outDataArray);

    neighborIndexArray[0] = 100;
    numNeighborsArray[0] = 101;
    outNeighborPosDensArray[0] = 105;
    outNeighborVelocArray[0] = 106;
    selfPosArray[0] = 102;
    selfVelocArray[0] = 103;
    selfDensArray[0] = 104;

    ga.setSelfArrays(selfPosArray, selfVelocArray, selfDensArray);
    ga.runTest();
    std::cerr << outDataArray[0] << " " << outDataArray[1] << " " << outDataArray[2] << " " << outDataArray[3] << " " << outDataArray[4] << " " << outDataArray[5] << " " << outDataArray[6] << " " << outDataArray[7];

    selfPosArray[1] = 202;
    selfVelocArray[1] = 203;
    selfDensArray[1] = 204;

    ga.setSelfArrays(selfPosArray + 1, selfVelocArray + 1, selfDensArray + 1);
    ga.runTest();
    std::cerr << outDataArray[0] << " " << outDataArray[1] << " " << outDataArray[2] << " " << outDataArray[3] << " " << outDataArray[4] << " " << outDataArray[5] << " " << outDataArray[6] << " " << outDataArray[7];

    ga.cleanUp();
    //DOUBLES_EQUAL(5.0, ps.maxVelicty(), 1.0E-3);
}
