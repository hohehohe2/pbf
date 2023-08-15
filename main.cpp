#include <math.h>
#include <iostream>
#include "particles.h"
#include "sph.h"
#include "unitConversion.h"

/** \mainpage

\author Koichi Tamura

<br>
<br>
<br>

[Summary]
<br>
<br>
SPH fluid implementation.<br>
<br>
Reference<br>
- Particle-Based Fluid Simulation for Interactive Applications<br>
  Matthias Müller, David Charypar and Markus Gross<br>
- Optimized Spatial Hashing for Collision Detection of Deformable Objects<br>
  Matthias Teschner, Bruno Heidelberger, Matthias Müller, Danat Pomeranets and Markus Gross
- 粒子法シミュレーション―物理ベースCG入門
  越塚 誠一 (著) 

*/



using namespace hohe2;

void init(Particles& ps)
{

    //Fluid particles.
    for (int x = -25; x < -5; ++x)
    {
        for (int y = -20; y < 0; ++y)
        {
            for (int z = -10; z < 10; ++z)
            {
                ps.p.push_back(x);
                ps.p.push_back(y);
                ps.p.push_back(z);
                ps.isWall.push_back(0);
            }
        }
    }

    //Wall particles.
    for (int x = -30; x < 31; ++x)
    {
        for (int y = -30; y < 31; ++y)
        {
            for (int z = -15; z < 16; ++z)
            {
                if (
                    (-28 < x &&  x < 28) &&
                    (-28 < y &&  y < 28) &&
                    (-13 < z &&  z < 13))
                {
                    continue;
                }
                if (y >= 28)
                {
                    continue;
                }

                ps.p.push_back(x);
                ps.p.push_back(y);
                ps.p.push_back(z);
                ps.isWall.push_back(1);
            }
        }
    }

}

void write(const Particles& ps, float t)
{
    std::cout << t << std::endl << ps << std::endl;
}

int main()
{
    UnitConversion uConv;
    uConv.setUnitLengthInMeters(0.01);
    uConv.setUnitTimeInSeconds(0.1);
    Sph sph(uConv);
    Particles ps;
    init(ps);
    sph.addParticles(ps.p.data(), ps.getNumParticles());
    ps.confirm();
    float deltaT = 0.1f; //Time step in the specified time unit.
    float t;
    for(t = 0.0f; t < 15.0f; t += deltaT)
    {
        write(ps, t);
        std::cerr << t << std::endl;
        sph.step(ps, deltaT);
    }
    write(ps, t);
    return 0;
}
