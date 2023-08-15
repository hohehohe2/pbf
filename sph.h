#ifndef HOHE2_sph_H
#define HOHE2_sph_H

#include <vector>
#include "particles.h"
#include "spacialHash.h"
#include "unitConversion.h"
#include "gpuAccessor.h"

namespace hohe2
{

///Sph calculation engine.
class Sph
{
public:

    //-------These values are unit independent-------

    ///Courant number (so called, actually not).
    static const float C = 0.1f; //TODO: Needs adjustment.

    ///Core radius of w;
    static const float RE = 2.5f; //TODO: Needs adjustment.

    ///Rest density.
    static const float DENSITY0 = 1.0f;

    //Both Gas constant and dynamic viscosity is unit dependent but its too much dependent on
    //simulation method, especially the gas constant is infinite for inviscos fluids
    //and we virtually introduce it for the explicit solution.
    ///Gas const.

    static const float K = 20000.0f;

    ///Dynamic viscosity.
    static const float MU = 10.0;

    ///Max velocity.
    static const float MAX_VELOCITY = 5000000;

    ///Max acceleration.
    static const float MAX_ACCELERATION = 0.5;


    //-------Unit dependent constants-------

    ///Default gravity in MTS(meter, ton, sec) = -9.8.
    static const float DEFAULTG = -9.8f;

    ///Number of neighbor hash cells.
    static const int NUM_EVERY_NEIGHBOR_CELLS = 27;

    ///Image channel depth for the GPU.
    static const int CHANNELDEPTH = 4;


    static const int NUM_NEIGHBOR_CELLS = 1;
    static const int NUMBLOCKS = 1;
    static const int NUMTHREADS = 32 * 4;
    static const int NUMITERATION = 4;

private:
    GpuAccessor ga_;

    UnitConversion& uConv_;

    //Gravity.
    float G_;

    //TODO: Delete it when we remove kd-tree related code.
    SpacialHash spacialHash_;
    SpacialHash spacialHashW_;

public:
    ///Constructor.
    /**
       @param [in] uConv Unit conversion object.
    */
    Sph(UnitConversion& uConv) : uConv_(uConv)
    {
        G_ = uConv.accelerationFromMts(DEFAULTG);
        //ga_.
    }

    ///Set Unit Length.
    /**
       @param [in] unitLengthInMeters Length unit size in meters.
       @return None.
     */
    void setUnitLengthInMeters(float unitLengthInMeters);

    ///Set Unit Time.
    /**
       @param [in] unitTimeInSeconds Time unit size in seconds.
       @return None.
     */
    void setUnitTimeInSeconds(float unitTimeInSeconds);

    ///Set particle data.
    /**
       @param [in] positions Particle position data.
       @param [in] numParticles Number of particles to add.
       @return None.
     */
    void addParticles(const float* positions, int numParticles);

    ///Proceed the simulation with deltaT.
    /**
       @param [in, out] particles Particle data to calculate SPH on.
       @param [in] deltaT Time step.
       @return false on failure.
    */
    bool step(Particles& particles, float deltaT);

private:
    bool createIndexArray_(const Particles& particles, int startPid,
                          int* neighborIndexArray, int* numNeighborsArray,
                          SpacialHash& neighborSHash,
                          const float* inNeighborPosArray=NULL, float* outNeighborPosDensArray=NULL,
                          const float* inNeighborVelocArray=NULL, float* outNeighborVelocArray=NULL,
                          const float* inNeighborDensArray=NULL);
    float w_(float dist2) const;
    void gradW_(const float r[3], float result[3]) const;
    float laplaceW_(const float r[3]) const;
    void calcParticleDensity_(Particles& particles);
    void calcParticleDensityOne_(Particles& particles, SpacialHash& shash, BaseParticles& neighborParticles);
    template < typename T > void setMinimumDensity_(T& particles);
    void calcParticleForce_(Particles& particles);
    void calcParticleVelocity_(Particles& particles, float dt);
    void calcParticlePos_(Particles& particles, float dt);
    void pressureForce_(Particles& particles);
    void pressureForceOne_(Particles& particles, SpacialHash& shash, BaseParticles& neighborParticles);
    void viscosityForce_(Particles& particles);
    void viscosityForceOneFluid_(Particles& particles, SpacialHash& shash, Particles& neighborParticles);
    void externalForce_(Particles& particles);

    void calcParticleDensityCpu_(int processSize, int* neighborIndexArray, int* numNeighborsArray,
                                 float* selfPosArray, float* outNeighborPosDensArray,
                                 float* outDataArray);
    void pressureForceCpu_(int processSize, int* neighborIndexArray, int* numNeighborsArray,
                           float* selfPosArray, float* selfDensArray, float* outNeighborPosDensArray, float* outDataArray);
    void viscosityForceCpuFluid_(int processSize, int* neighborIndexArray, int* numNeighborsArray,
                                 float* selfPosArray, float* selfVelocArray,
                                 float* outNeighborPosDensArray, float* outNeighborVelocArray,
                                 float* outDataArray);

    void calcParticleDensityGpu_(int processSize, int* neighborIndexArray, int* numNeighborsArray,
                                 float* selfPosArray, float* outNeighborPosDensArray,
                                 float* outDataArray);
    void pressureForceGpu_(int processSize, int* neighborIndexArray, int* numNeighborsArray,
                           float* selfPosArray, float* selfDensArray, float* outNeighborPosDensArray, float* outDataArray);
    void viscosityForceGpuFluid_(int processSize, int* neighborIndexArray, int* numNeighborsArray,
                                 float* selfPosArray, float* selfVelocArray,
                                 float* outNeighborPosDensArray, float* outNeighborVelocArray,
                                 float* outDataArray);
    void adjustPosition_(Particles& particles);
    void adjustVelocity_(Particles& particles, float dt);

    void calcViscosity_(Particles& particles, float dt);
    void setFinalPosition_(Particles& particles, float dt);
};

}
#endif
