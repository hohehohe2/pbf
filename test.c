const sampler_t s_nearest = CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE;

__kernel void
test(
    __global int numParticles,
    __global int* neighborIndex,
    __global int* numNeighbors,
    __global float* selfPos,
    __global float* selfVeloc,
    __global float* selfDens,
    __read_only image2d_t imagePosDens,
    __read_only image2d_t imageVeloc,
    __global __write_only float* outData
    )
{
    unsigned int gid = get_global_id(0);
    unsigned int gSize = get_global_size(0);
    unsigned int width = get_image_width(imagePosDens);
    unsigned int height = get_image_height(imagePosDens);
    outData[0] = numParticles;
    outData[1] = neighborIndex[0];
    outData[2] = numNeighbors[0];
    outData[3] = selfPos[0];
    outData[4] = selfVeloc[0];
    outData[5] = selfDens[0];
    outData[6] = read_imagef(imagePosDens, s_nearest, (int2)(0, 0)).x;
    outData[7] = read_imagef(imageVeloc, s_nearest, (int2)(0, 0)).x;

/*
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; x += gSize)
        {
            float4 texel = read_imagef(imageIn, s_nearest, (int2)(x + gid, y));
            //write_imagef(imageOut, (int2)(x + gid, y), (float4)(texel.x, texel.y, texel.x + texel.y, texel.w));
            //write_imagef(imageOut, (int2)(x + gid, y), (float4)(texel.x, texel.y, texel.x + texel.y, texel.w));
            write_imagef(imageOut, (int2)(x + gid, y), amobj[0]);
        }
    }
*/
}

__kernel void density(
    __global int numParticles,
    __global int* neighborIndex,
    __global int* numNeighbors,
    __global float* selfPos,
    __read_only image2d_t imagePosDens,
    __global __write_only float* outData
)
{
}

__kernel void pressure(
    __global int numParticles,
    __global int* neighborIndex,
    __global int* numNeighbors,
    __global float* selfPos,
    __global float* selfDens,
    __read_only image2d_t imagePosDens,
    __global __write_only float* outData
)
{
}

__kernel void viscosityFluid(
    __global int numParticles,
    __global int* neighborIndex,
    __global int* numNeighbors,
    __global float* selfPos,
    __global float* selfVeloc,
    __read_only image2d_t imagePosDens,
    __read_only image2d_t imageVeloc,
    __global __write_only float* outData
)
{
}

__kernel void viscosityWall(
    __global int numParticles,
    __global int* neighborIndex,
    __global int* numNeighbors,
    __global float* selfPos,
    __global float* selfVeloc,
    __read_only image2d_t imagePosDens,
    __global __write_only float* outData
)
{
}
