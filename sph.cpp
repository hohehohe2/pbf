#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include "sph.h"

using namespace hohe2;

const int Sph::NUM_EVERY_NEIGHBOR_CELLS;

const float Sph::C;
const float Sph::RE;
const float Sph::K;
const float Sph::MU;
const float Sph::DENSITY0;
const float Sph::DEFAULTG;

const float Sph::MAX_ACCELERATION;
const float Sph::MAX_VELOCITY;

const int Sph::NUM_NEIGHBOR_CELLS;
const int Sph::NUMTHREADS;
const int Sph::NUMBLOCKS;
const int Sph::NUMITERATION;
const int Sph::CHANNELDEPTH;


//TODO: Adhoc code!. 
static float* outDataArray = NULL;
static float* outNeighborPosDensArray=NULL;
static float* outNeighborVelocArray=NULL;
static int* neighborIndexArray = NULL;
static int* numNeighborsArray = NULL;


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::setUnitLengthInMeters(float unitLengthInMeters)
{
    uConv_.setUnitLengthInMeters(unitLengthInMeters);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::setUnitTimeInSeconds(float unitTimeInSeconds)
{
    uConv_.setUnitTimeInSeconds(unitTimeInSeconds);

}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::addParticles(const float* positions, int numParticles)
{
    spacialHash_.addParticles(positions, numParticles);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool Sph::step(Particles& particles, float deltaT)
{
    if (! particles.confirm(false))
    {
        return false;
    }

    //TODO: Adhoc code!.
    if ( ! outNeighborPosDensArray)
    {
        ga_.initlalize("test.c", NUMBLOCKS, NUMTHREADS, NUMITERATION);
        ga_.getBuffers(neighborIndexArray, numNeighborsArray,
                       outNeighborPosDensArray, outNeighborVelocArray,
                       outDataArray);
//         neighborIndexArray = new int[NUMITERATION * NUMTHREADS * NUM_EVERY_NEIGHBOR_CELLS];
//         numNeighborsArray = new int[NUMITERATION * NUMTHREADS * NUM_EVERY_NEIGHBOR_CELLS];
//         outNeighborPosDensArray = new float[NUMITERATION * NUMTHREADS * NUM_EVERY_NEIGHBOR_CELLS * 8 * CHANNELDEPTH]; //TODO: May not be enough. How can we know the enough size?
//         outNeighborVelocArray= new float[NUMITERATION * NUMTHREADS * NUM_EVERY_NEIGHBOR_CELLS * 8 * CHANNELDEPTH]; //TODO: May not be enough. How can we know the enough size?
//         outDataArray = new float[NUMITERATION * NUMTHREADS * 3];
    }

    spacialHash_.restructure(particles.p.data());

    float remain = deltaT;
    bool loop = true;
    do
    {
        //Iteration time step.
        float dt = C / particles.maxVelocity();
        dt = 0.02;
        if (dt > remain)
        {
            dt = remain;
            loop = false;
        }
        remain -= dt;

        calcParticleForce_(particles);
        calcParticleVelocity_(particles, dt); //u <- u + a * dt.
        calcParticlePos_(particles, dt);      //ppos <- pos, pos <- pos + u * dt.

        for (int i = 0; i < 3; ++i)
        {
            calcParticleDensity_(particles);  //update d.
            setMinimumDensity_(particles);    //update d.
            adjustPosition_(particles);       //npos <- pos - (adjustment), pos <- npos.
        }
        adjustVelocity_(particles, dt);       //u <- (pos - ppos) / dt.
        calcViscosity_(particles, dt);
        setFinalPosition_(particles, dt);

    } while(loop && remain > 1.0e-5f);

    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool Sph::createIndexArray_(const Particles& particles, int startPid,
                            int* neighborIndexArray, int* numNeighborsArray,
                            SpacialHash& neighborSHash,
                            const float* inNeighborPosArray, float* outNeighborPosDensArray,
                            const float* inNeighborVelocArray, float* outNeighborVelocArray,
                            const float* inNeighborDensArray)
{
    //Get a vector of every cell and mark them as "unprocessed".
    std::vector < const HashCell* > cells;
    neighborSHash.getEveryCells(cells);
    int numCells = cells.size();
    for (int i = 0; i < numCells; ++i)
    {
        cells[i]->setMark(-1);
    }

    //For each partcile, find adjacent cells and neighbor particle nids in it.
    int numParticles = particles.getNumParticles();
    int pCount = 0; //Index in the output particle data arrays, i.e. neighborIndexArray and numNeighborsArray.
    int neighborIndex = 0; //Index in the output neighbor data arrays. i.e. outNeighbor*.
    for (int iter = 0; iter < NUMITERATION; ++iter)
    {
        for (int th = 0; th < NUMTHREADS; ++th)
        {
            //Done every particles. Let's fill the rest with dummy data and exit the method.
            int pid = startPid + iter * NUMTHREADS + th;
            if (pid >= numParticles)
            {
                //Fill with dummy data.
                //TODO: Do not have to fill every allocated memory with 0, only ones that are used will do.
                memset(numNeighborsArray + pCount, 0, (NUMITERATION * NUMTHREADS * NUM_EVERY_NEIGHBOR_CELLS - pCount) * sizeof(int));
                return false;
            }

            //Iterate over every adjacent cells that contains the particle pid.
            //TODO: Output arrays must have enough memory allocated, how do we know the size?
            short cx, cy, cz;
            particles.getCellIndexes(pid, cx, cy, cz);
            for (int icx = -NUM_NEIGHBOR_CELLS; icx <= NUM_NEIGHBOR_CELLS; ++icx)
            {
                for (int icy = -NUM_NEIGHBOR_CELLS; icy <= NUM_NEIGHBOR_CELLS; ++icy)
                {
                    for (int icz = -NUM_NEIGHBOR_CELLS; icz <= NUM_NEIGHBOR_CELLS; ++icz)
                    {
                        HashCell* hc = neighborSHash.getCell(cx + icx, cy + icy, cz + icz);
                        if ( ! hc)
                        {
                            //neighborIndexArray[pCount] = 0;
                            numNeighborsArray[pCount++] = 0;
                            continue;
                        }

                        int mark = hc->getMark();
                        if (mark >= 0)
                        {
                            //Neighbor particles for the cell already added. We don't have to add them again.
                            neighborIndexArray[pCount] = neighborIndexArray[mark];
                            numNeighborsArray[pCount++] = numNeighborsArray[mark];
                            continue;
                        }
                        //Mark the cell as 'processed", with pCount so that neighbor index and num neighbor particles can be copied next time.
                        hc->setMark(pCount);

                        //Neighbor particle ids in the cell and number of them.
                        const int* nids = hc->getParticleIds();
                        int numNeighbors = hc->getNumParticles();

                        //Set the start index and number of the neighbor particles data.
                        neighborIndexArray[pCount] = neighborIndex;
                        numNeighborsArray[pCount++] = numNeighbors;

                        //Copy neighbor particle parameters in the cell.
                        for (int i = 0; i < numNeighbors; ++i)
                        {
                            int nid = nids[i];

                            if (outNeighborPosDensArray)
                            {
                                const float* npos = inNeighborPosArray + nid * 3;
                                outNeighborPosDensArray[neighborIndex * CHANNELDEPTH + 0] = npos[0];
                                outNeighborPosDensArray[neighborIndex * CHANNELDEPTH + 1] = npos[1];
                                outNeighborPosDensArray[neighborIndex * CHANNELDEPTH + 2] = npos[2];
                            }
                            if (outNeighborVelocArray)
                            {
                                const float* nvelocity = inNeighborVelocArray + nid * 3;
                                outNeighborVelocArray[neighborIndex * CHANNELDEPTH + 0] = nvelocity[0];
                                outNeighborVelocArray[neighborIndex * CHANNELDEPTH + 1] = nvelocity[1];
                                outNeighborVelocArray[neighborIndex * CHANNELDEPTH + 2] = nvelocity[2];
                            }
                            if (inNeighborDensArray)
                            {
                                outNeighborPosDensArray[neighborIndex * CHANNELDEPTH + 3] = inNeighborDensArray[nid];
                            }
                            ++neighborIndex;
                        }
                    }
                }
            }
        }
    }
    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
float Sph::w_(float dist2) const
{
    //Particle-Based Fluid Simulation for Interactive Applications.
    const float re9 = RE * RE * RE * RE * RE * RE * RE * RE * RE;
    if (dist2 < RE * RE)
    {
        float l = RE * RE - dist2;
        return 315.0f / 64.0f / M_PI / re9 * l * l * l;
    }
    else
    {
        return 0.0f;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::gradW_(const float r[3], float result[3]) const
{
    const float re6 = RE * RE * RE * RE * RE * RE;
    float nn = r[0] * r[0] + r[1] * r[1] + r[2] * r[2];
    if (nn < 1.0E-8)
    {
        //Same particle. Ignore.
        result[0] = 0.0f;
        result[1] = 0.0f;
        result[2] = 0.0f;
        return;
    }
    float n = sqrt(nn);
    if (n < RE)
    {
        float l = RE - n;
        float coef = -45.0f / M_PI / re6 * l * l / n;
        result[0] = coef * r[0];
        result[1] = coef * r[1];
        result[2] = coef * r[2];
    }
    else
    {
        result[0] = 0.0f;
        result[1] = 0.0f;
        result[2] = 0.0f;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
float Sph::laplaceW_(const float r[3]) const
{
    const float re6 = RE * RE * RE * RE * RE * RE;
    float n = sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
    if (n < 1.0E-4)
    {
        //Same particle. Ignore.
        return 0.0f;
    }
    if (n < RE)
    {
        float l = RE - n;
        return 45.0f / M_PI / re6 * l;
    }
    else
    {
        return 0.0f;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::calcParticleDensity_(Particles& particles)
{
    //Fill the result density with zero.
    memset(particles.d.data(), 0, particles.getNumParticles() * 1 * sizeof(float)); //1 is dimension.

    //Density from neighbor fluid particles.
    calcParticleDensityOne_(particles, spacialHash_, particles);
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::calcParticleDensityOne_(Particles& particles, SpacialHash& shash, BaseParticles& neighborParticles)
{
    bool continued;
    int startPid = 0;
    do
    {
        continued = createIndexArray_(particles, startPid,
                                      neighborIndexArray, numNeighborsArray,
                                      shash,
                                      neighborParticles.p.data(), outNeighborPosDensArray);
        float* selfPosArray = particles.p.data() + startPid * 3;

        //Create and initialize the GPU result buffer.
        memset(outDataArray, 0, NUMITERATION * NUMTHREADS * 1 * sizeof(float)); //1 is dimension.

        //GPU code.
        int processSize = particles.getNumParticles() - startPid;
        if (processSize > NUMITERATION * NUMTHREADS)
        {
            processSize = NUMITERATION * NUMTHREADS;
        }
        calcParticleDensityCpu_(processSize, neighborIndexArray, numNeighborsArray, selfPosArray, outNeighborPosDensArray, outDataArray);

        //Back to the host.
        bool end = false;
        for (int iter = 0; iter < NUMITERATION; ++iter)
        {
            for (int th = 0; th < NUMTHREADS; ++th)
            {
                int relativePid = iter * NUMTHREADS + th;
                if (startPid + relativePid >= particles.getNumParticles())
                {
                    end = true;
                    break;
                }
                particles.d[startPid + relativePid] += outDataArray[relativePid];
            }
            if (end)
            {
                break;
            }
        }

        startPid += NUMITERATION * NUMTHREADS;
    } while(continued);
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
template < typename T > void Sph::setMinimumDensity_(T& particles)
{
    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {
        float pdens = particles.d[pid];
        if (pdens < DENSITY0)
        {
            particles.d[pid] = DENSITY0;
        }
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::calcParticleForce_(Particles& particles)
{
    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {
        float* force = particles.f.data() + pid * 3;
        force[0] = 0;
        force[1] = 0;
        force[2] = 0;
    }
    externalForce_(particles);
    //viscosityForce_(particles);
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::calcViscosity_(Particles& particles, float dt)
{
    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {
        if (particles.isWall.data()[pid])
        {
            continue;
        }

        float* force = particles.f.data() + pid * 3;
        force[0] = 0;
        force[1] = 0;
        force[2] = 0;
    }
    viscosityForce_(particles);

    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {
        float* velocity = particles.u.data() + pid * 3;
        float* force = particles.f.data() + pid * 3;

        if (particles.isWall.data()[pid])
        {
            continue;
        }

        float accel[3];
        accel[0] = force[0] * dt;
        accel[1] = force[1] * dt;
        accel[2] = force[2] * dt;
        float a2 = accel[0] * accel[0] + accel[1] * accel[1] + accel[2] * accel[2];
        if (a2 > MAX_ACCELERATION * MAX_ACCELERATION)
        {
            float s = MAX_ACCELERATION / sqrt(a2);
            accel[0] *= s;
            accel[1] *= s;
            accel[2] *= s;
        }

        velocity[0] += accel[0];
        velocity[1] += accel[1];
        velocity[2] += accel[2];
        float v2 = velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2];

        if (v2 > MAX_VELOCITY * MAX_VELOCITY)
        {
            float s = MAX_VELOCITY / sqrt(v2);
            velocity[0] *= s;
            velocity[1] *= s;
            velocity[2] *= s;
        }

    }

}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::calcParticleVelocity_(Particles& particles, float dt)
{
    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {
        float* velocity = particles.u.data() + pid * 3;
        float* force = particles.f.data() + pid * 3;

        if (particles.isWall.data()[pid])
        {
            velocity[0] = 0.0f;
            velocity[1] = 0.0f;
            velocity[2] = 0.0f;
            continue;
        }

        float accel[3];
        accel[0] = force[0] * dt;
        accel[1] = force[1] * dt;
        accel[2] = force[2] * dt;
        float a2 = accel[0] * accel[0] + accel[1] * accel[1] + accel[2] * accel[2];
        if (a2 > MAX_ACCELERATION * MAX_ACCELERATION)
        {
            float s = MAX_ACCELERATION / sqrt(a2);
            accel[0] *= s;
            accel[1] *= s;
            accel[2] *= s;
        }

        velocity[0] += accel[0];
        velocity[1] += accel[1];
        velocity[2] += accel[2];
        float v2 = velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2];

        if (v2 > MAX_VELOCITY * MAX_VELOCITY)
        {
            float s = MAX_VELOCITY / sqrt(v2);
            velocity[0] *= s;
            velocity[1] *= s;
            velocity[2] *= s;
        }

    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::externalForce_(Particles& particles)
{
    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {
        float* force = particles.f.data() + pid * 3;
        force[0] += 0.0f;
        force[1] += G_;
        force[2] += 0.0f;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::calcParticleDensityCpu_(int processSize, int* neighborIndexArray, int* numNeighborsArray,
                                  float* selfPosArray, float* outNeighborPosDensArray,
                                  float* outDataArray)
{
    for (int iter = 0; iter < NUMITERATION; ++iter)
    {
        for (int th = 0; th < NUMTHREADS; ++th)
        {
            int relativePid = iter * NUMTHREADS + th;
            if (relativePid > processSize)
            {
                continue;
            }

            float* pos = selfPosArray + relativePid * 3;
            float density = 0.0f;
            for (int c = 0; c < NUM_EVERY_NEIGHBOR_CELLS; ++c)
            {
                int neighborIndex = neighborIndexArray[relativePid * NUM_EVERY_NEIGHBOR_CELLS+ c];
                int numNeighbors = numNeighborsArray[relativePid * NUM_EVERY_NEIGHBOR_CELLS+ c];
                for (int nn = 0; nn < numNeighbors; ++nn)
                {
                    float* npos = outNeighborPosDensArray + (neighborIndex + nn) * CHANNELDEPTH;
                    float dist[3];
                    dist[0] = pos[0] - npos[0];
                    dist[1] = pos[1] - npos[1];
                    dist[2] = pos[2] - npos[2];
                    float dist2 = dist[0] * dist[0] + dist[1] * dist[1] + dist[2] * dist[2];
                    density += w_(dist2);
                }
            }
            outDataArray[relativePid] += density;
        }
    }
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::viscosityForce_(Particles& particles)
{
    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {
        if (particles.isWall.data()[pid])
        {
            continue;
        }

        float* pos = particles.p.data() + pid * 3;
        float* pvelocity = particles.u.data() + pid * 3;
        float* force = particles.f.data() + pid * 3;

        short cx, cy, cz;
        particles.getCellIndexes(pid, cx, cy, cz);
        for (int icx = -NUM_NEIGHBOR_CELLS; icx <= NUM_NEIGHBOR_CELLS; ++icx)
        {
            for (int icy = -NUM_NEIGHBOR_CELLS; icy <= NUM_NEIGHBOR_CELLS; ++icy)
            {
                for (int icz = -NUM_NEIGHBOR_CELLS; icz <= NUM_NEIGHBOR_CELLS; ++icz)
                {
                    HashCell* hc = spacialHash_.getCell(cx + icx, cy + icy, cz + icz);
                    if ( ! hc)
                    {
                        continue;
                    }
                    const int* nids = hc->getParticleIds();
                    for (int i = 0; i < hc->getNumParticles(); ++i)
                    {
                        int nid = nids[i];

                        if (particles.isWall.data()[nid])
                        {
                            continue;
                        }

                        float* npos = particles.p.data() + nid * 3;
                        float* nvelocity = particles.u.data() + nid * 3;
                        float* ndens = particles.d.data() + nid;

                        float dist[3];
                        dist[0] = pos[0] - npos[0];
                        dist[1] = pos[1] - npos[1];
                        dist[2] = pos[2] - npos[2];

                        float laplace = laplaceW_(dist);
                        float viscosCoef = MU / (*ndens) * laplace;
                        force[0] += viscosCoef * (nvelocity[0] - pvelocity[0]);
                        force[1] += viscosCoef * (nvelocity[1] - pvelocity[1]);
                        force[2] += viscosCoef * (nvelocity[2] - pvelocity[2]);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::viscosityForceCpuFluid_(int processSize, int* neighborIndexArray, int* numNeighborsArray,
                                  float* selfPosArray, float* selfVelocArray,
                                  float* outNeighborPosDensArray, float* outNeighborVelocArray,
                                  float* outDataArray)
{
    for (int iter = 0; iter < NUMITERATION; ++iter)
    {
        for (int th = 0; th < NUMTHREADS; ++th)
        {
            int relativePid = iter * NUMTHREADS + th;
            if (relativePid > processSize)
            {
                continue;
            }
            float* pos = selfPosArray + relativePid * 3;
            float* pvelocity = selfVelocArray + relativePid * 3;

            float force[3] = {0, 0, 0};
            for (int c = 0; c < NUM_EVERY_NEIGHBOR_CELLS; ++c)
            {
                int neighborIndex = neighborIndexArray[relativePid * NUM_EVERY_NEIGHBOR_CELLS + c];
                int numNeighbors = numNeighborsArray[relativePid * NUM_EVERY_NEIGHBOR_CELLS + c];
                for (int nn = 0; nn < numNeighbors; ++nn)
                {
                    float* npos = outNeighborPosDensArray + (neighborIndex + nn) * CHANNELDEPTH;
                    float* nvelocity = outNeighborVelocArray + (neighborIndex + nn) * CHANNELDEPTH;
                    float* ndens = npos + 3;
                    float dist[3];
                    dist[0] = pos[0] - npos[0];
                    dist[1] = pos[1] - npos[1];
                    dist[2] = pos[2] - npos[2];
                    float laplace = laplaceW_(dist);
                    float viscosCoef = MU / (*ndens) * laplace;
                    force[0] += viscosCoef * (nvelocity[0] - pvelocity[0]);
                    force[1] += viscosCoef * (nvelocity[1] - pvelocity[1]);
                    force[2] += viscosCoef * (nvelocity[2] - pvelocity[2]);
                }
            }
            outDataArray[relativePid * 3 + 0] += force[0];
            outDataArray[relativePid * 3 + 1] += force[1];
            outDataArray[relativePid * 3 + 2] += force[2];
        }
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::adjustPosition_(Particles& particles)
{
    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {

        if (particles.isWall.data()[pid])
        {
            continue;
        }

        float* pos = particles.p.data() + pid * 3;
        float pdens = particles.d[pid];
        float* nextPos = particles.np.data() + pid * 3;
        float& scalingFactor = particles.s[pid];

        nextPos[0] = pos[0];
        nextPos[1] = pos[1];
        nextPos[2] = pos[2];

        if (!pdens)
        {
            continue;
        }

        float sumGradLenJ2 = 0.0f;
        short cx, cy, cz;
        particles.getCellIndexes(pid, cx, cy, cz);
        for (int icx = -NUM_NEIGHBOR_CELLS; icx <= NUM_NEIGHBOR_CELLS; ++icx)
        {
            for (int icy = -NUM_NEIGHBOR_CELLS; icy <= NUM_NEIGHBOR_CELLS; ++icy)
            {
                for (int icz = -NUM_NEIGHBOR_CELLS; icz <= NUM_NEIGHBOR_CELLS; ++icz)
                {
                    HashCell* hc = spacialHash_.getCell(cx + icx, cy + icy, cz + icz);
                    if ( ! hc)
                    {
                        continue;
                    }
                    const int* nids = hc->getParticleIds();
                    for (int i = 0; i < hc->getNumParticles(); ++i)
                    {
                        int nid = nids[i];
                        float* npos = particles.p.data() + nid * 3;
                        float dist[3];
                        dist[0] = pos[0] - npos[0];
                        dist[1] = pos[1] - npos[1];
                        dist[2] = pos[2] - npos[2];
                        float gradw[3];
                        gradW_(dist, gradw);
                        sumGradLenJ2 += gradw[0] * gradw[0] + gradw[1] * gradw[1] + gradw[2] * gradw[2];
                    }
                }
            }
        }

        //--Scaling factor;
        float nominator = pdens / DENSITY0 - 1.0f;
        float denominator = sumGradLenJ2 / DENSITY0 / DENSITY0;
        float epsilon = 0.3f;
        scalingFactor = nominator / (denominator + epsilon);

    }

    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {

        if (particles.isWall.data()[pid])
        {
            continue;
        }

        float* pos = particles.p.data() + pid * 3;
        float* nextPos = particles.np.data() + pid * 3;
        float& scalingFactor = particles.s[pid];

        if (! scalingFactor)
        {
            continue;
        }

        short cx, cy, cz;
        particles.getCellIndexes(pid, cx, cy, cz);
        for (int icx = -NUM_NEIGHBOR_CELLS; icx <= NUM_NEIGHBOR_CELLS; ++icx)
        {
            for (int icy = -NUM_NEIGHBOR_CELLS; icy <= NUM_NEIGHBOR_CELLS; ++icy)
            {
                for (int icz = -NUM_NEIGHBOR_CELLS; icz <= NUM_NEIGHBOR_CELLS; ++icz)
                {
                    HashCell* hc = spacialHash_.getCell(cx + icx, cy + icy, cz + icz);
                    if ( ! hc)
                    {
                        continue;
                    }
                    const int* nids = hc->getParticleIds();
                    for (int i = 0; i < hc->getNumParticles(); ++i)
                    {

                        int nid = nids[i];
                        float* npos = particles.p.data() + nid * 3;
                        float& nscalingFactor = particles.s[nid];
                        float dist[3];
                        dist[0] = pos[0] - npos[0];
                        dist[1] = pos[1] - npos[1];
                        dist[2] = pos[2] - npos[2];

                        float gradw[3];
                        gradW_(dist, gradw);

                        float dist2 = dist[0] * dist[0] + dist[1] * dist[1] + dist[2] * dist[2];
                        float h = 2.5f;
                        float rw = w_(dist2) / w_(h * 0.1f * h * 0.1f);
                        float scorr = 0.3f * rw * rw * rw * rw;

                        nextPos[0] -= (scalingFactor + nscalingFactor + scorr) * gradw[0] / DENSITY0;
                        nextPos[1] -= (scalingFactor + nscalingFactor + scorr) * gradw[1] / DENSITY0;
                        nextPos[2] -= (scalingFactor + nscalingFactor + scorr) * gradw[2] / DENSITY0;

                    }
                }
            }
        }

    }

    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {

        if (particles.isWall.data()[pid])
        {
            continue;
        }

        float* pos = particles.p.data() + pid * 3;
        float* nextPos = particles.np.data() + pid * 3;
        pos[0] = nextPos[0];
        pos[1] = nextPos[1];
        pos[2] = nextPos[2];
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::calcParticlePos_(Particles& particles, float dt)
{
    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {

        if (particles.isWall.data()[pid])
        {
            continue;
        }

        float* pos = particles.p.data() + pid * 3;
        float* ppos = particles.pp.data() + pid * 3;
        float* velocity = particles.u.data() + pid * 3;

        ppos[0] = pos[0];
        ppos[1] = pos[1];
        ppos[2] = pos[2];

        pos[0] += velocity[0] * dt;
        pos[1] += velocity[1] * dt;
        pos[2] += velocity[2] * dt;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Sph::adjustVelocity_(Particles& particles, float dt)
{
//     std::cerr << std::endl;
    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {

        if (particles.isWall.data()[pid])
        {
            continue;
        }

        float* pos = particles.p.data() + pid * 3;
        float* ppos = particles.pp.data() + pid * 3;
        float* velocity = particles.u.data() + pid * 3;

//         if (true)
//         {
//             std::cerr << " " << pid << " " << velocity[0] << " " << pos[0] << " " << ppos[0] << " " << particles.maxVelocity() << std::endl;
//         }

        velocity[0] = (pos[0] - ppos[0]) / dt;
        velocity[1] = (pos[1] - ppos[1]) / dt;
        velocity[2] = (pos[2] - ppos[2]) / dt;
    }

}

void Sph::setFinalPosition_(Particles& particles, float dt)
{
    for (int pid = 0; pid < particles.getNumParticles(); ++pid)
    {

        if (particles.isWall.data()[pid])
        {
            continue;
        }

        float* pos = particles.p.data() + pid * 3;
        float* ppos = particles.pp.data() + pid * 3;
        float* velocity = particles.u.data() + pid * 3;

        pos[0] = ppos[0] + velocity[0] * dt;
        pos[1] = ppos[1] + velocity[1] * dt;
        pos[2] = ppos[2] + velocity[2] * dt;
    }
}
