#ifndef HOHE2_particles_H
#define HOHE2_particles_H

#include <math.h>
#include <vector>
#include <iostream>

namespace hohe2
{

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
///Base particle data container.
struct BaseParticles
{
    ///Particle poisitions.
    std::vector < float > p;

    ///Particle density.
    std::vector < float > d;

    ///Get the number of particles.
    /**
       @return Number of particles.
     */
    int getNumParticles() const{return p.size() / 3;}

    ///Get the index of the cell this particles is in.
    /**
       @param [in] pos Particle position.
       @param [out] cx Cell index.
       @param [out] cy Cell index.
       @param [out] cz Cell index.
       @return None
     */
    inline static void getCellIndexes(const float pos[3], short& cx, short& cy, short& cz);

    ///Get the index of the cell this particles is in.
    /**
       @param [in] pid Particle id.
       @param [out] cx Cell index.
       @param [out] cy Cell index.
       @param [out] cz Cell index.
       @return None
     */
    inline void getCellIndexes(int pid, short& cx, short& cy, short& cz) const;

protected:
    BaseParticles(){}
    virtual ~BaseParticles(){}
    template < typename T >
    void setData_(std::vector < T >& setv, const std::vector < T >& addv, unsigned size, const T defaultValue);
    template < typename T >
    bool fillDefault_(std::vector < T >& setv, const T defaultValue, bool modifyData, int dim);
};

void BaseParticles::getCellIndexes(const float pos[3], short& cx, short& cy, short& cz)
{
    //Make the cell size 2*2*2.
    cx = floor(pos[0] / 2);
    cy = floor(pos[1] / 2);
    cz = floor(pos[2] / 2);
}

void BaseParticles::getCellIndexes(int pid, short& cx, short& cy, short& cz) const
{
    getCellIndexes(p.data() + pid * 3, cx, cy, cz);
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
///Particle cloud data container.
struct Particles : public BaseParticles
{
    ///Temporary particle poisitions.
    std::vector < float > pp;

    ///Particle poisitions after constraint solve.
    std::vector < float > np;

    ///Particle forces.
    std::vector < float > f;

    ///Particle velocities.
    std::vector < float > u;

    ///Wall flag.
    std::vector < int > isWall;

    ///Scaling factor.
    std::vector < float > s;

    ///Particle id that has max velocity.
    int maxPid;

    ///Constructor.
    Particles(){}

    ///Destructor.
    virtual ~Particles(){}

    ///Clear the state.
    /**
       @return None.
     */
    void clear();

    ///Make sure every data is set consistently. Add deafult values if not.
    /**
       @param [in] modifyData Set true to make the state of the particles consistent.
       @return True if data consistent now.
     */
    bool confirm(bool modifyData=true);

    ///Add a particle cloud data at the end of existing data.
    /**
       @param [in] particles Particle cloud data to add.
       @return None.
     */
    void addParticles(const Particles& particles);

    ///Get the max velocity of the particles.
    /**
       @return Max velocity.
     */
    float maxVelocity();
};


}

///<< operator for Particles.
/**
   os [in, out] Output stream.
   ps [in] Particles object.
 */
std::ostream& operator<<(std::ostream& os, const hohe2::Particles& ps);

///>> operator for Particles. Particles are added to the current existing ones.
/**
   os [in, out] Input stream.
   ps [out] Particles object.
 */
std::istream& operator>>(std::istream& is, hohe2::Particles& ps);

#endif
