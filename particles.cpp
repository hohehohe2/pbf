#include <math.h>
#include "particles.h"

using namespace hohe2;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
template < typename T >
void BaseParticles::setData_(std::vector < T >& setv, const std::vector < T >& addv, unsigned size, const T defaultValue)
{
    if (addv.size() == size)
    {
        setv.insert(setv.end(), addv.begin(), addv.end());
    }
    else
    {
        setv.insert(setv.end(), size, defaultValue);
    }
    
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
template < typename T >
bool BaseParticles::fillDefault_(std::vector < T >& setv, const T defaultValue, bool modifyData, int dim)
{
    unsigned size = getNumParticles() * dim;
    if (setv.size() != size)
    {
        if ( ! modifyData)
        {
            return false;
        }
        setv.clear();
        setv.insert(setv.end(), size, defaultValue);
    }
    return true;
}









//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Particles::clear()
{
    p.clear();
    pp.clear();
    np.clear();
    f.clear();
    u.clear();
    d.clear();
    s.clear();
    isWall.clear();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool Particles::confirm(bool modifyData)
{
    bool result = true;
    result = result && fillDefault_(pp, 0.0f, modifyData, 3);
    result = result && fillDefault_(np, 0.0f, modifyData, 3);
    result = result && fillDefault_(f, 0.0f, modifyData, 3);
    result = result && fillDefault_(u, 0.0f, modifyData, 3);
    result = result && fillDefault_(d, 0.0f, modifyData, 1);
    result = result && fillDefault_(s, 0.0f, modifyData, 1);
    result = result && fillDefault_(isWall, 0, modifyData, 1);
    return result;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Particles::addParticles(const Particles& particles)
{
    int size = particles.getNumParticles();
    p.insert(p.end(), particles.p.begin(), particles.p.end());
    setData_(pp, particles.pp, size * 3, 0.0f);
    setData_(np, particles.np, size * 3, 0.0f);
    setData_(f, particles.f, size * 3, 0.0f);
    setData_(u, particles.u, size * 3, 0.0f);
    setData_(d, particles.d, size, 0.0f);
    setData_(s, particles.s, size, 0.0f);
    setData_(isWall, particles.isWall, size, 0);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
float Particles::maxVelocity()
{
    //Give some minimum value to avoid div by zero error in the code that uses this method.
    float max = 1.0E-16f;
    for (int pid = 0; pid < getNumParticles(); ++pid)
    {
        float x = u[pid * 3];
        float y = u[pid * 3 + 1];
        float z = u[pid * 3 + 2];
        float sn = x * x + y * y + z * z;
        if (sn > max)
        {
            max = sn;
            maxPid = pid;
        }
    }
    return sqrt(max);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const Particles& ps)
{
    int numParticles = ps.getNumParticles();
    os << numParticles << std::endl;
    for (int pid = 0; pid < numParticles; ++pid)
    {
        os << std::fixed << ps.p[pid * 3 + 0] << " " << ps.p[pid * 3 + 1] << " " << ps.p[pid * 3 + 2] << " " << ps.isWall[pid];
        //TODO: Just for convenience.
        //os << std::fixed << ps.u[pid * 3 + 0] << " " << ps.u[pid * 3 + 1] << " " << ps.u[pid * 3 + 2] << " ";
        //os << std::fixed << ps.d[pid];
        os << std::endl;
    }
    return os;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
std::istream& operator>>(std::istream& is, hohe2::Particles& ps)
{
    int numParticles;
    is >> numParticles;
    for (int pid = 0; pid < numParticles; ++pid)
    {
        float px, py, pz, ux, uy, uz, pd;
        int isWall;
        is >> px >> py >> pz >> isWall;
        //TODO: Just for convenience.
        //is >> ux >> uy >> uz;
        //is >> pd;
        ps.p.push_back(px);
        ps.p.push_back(py);
        ps.p.push_back(pz);
        ps.u.push_back(ux);
        ps.u.push_back(uy);
        ps.u.push_back(uz);
        ps.d.push_back(pd);
        ps.s.push_back(pd);
        ps.isWall.push_back(isWall);
    }

    ps.f.insert(ps.f.end(), numParticles * 3, 0.0f);
    return is;
}
