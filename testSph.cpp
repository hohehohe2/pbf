#include <iostream>
#include <CppUnitLite/TestHarness.h>
#include "particles.h"
#include "sph.h"
#include "unitConversion.h"

using namespace hohe2;

static void pushP(Particles& ps, float x, float y, float z)
{
    ps.p.push_back(x);
    ps.p.push_back(y);
    ps.p.push_back(z);
}

static void pushP(WallParticles& ps, float x, float y, float z)
{
    ps.p.push_back(x);
    ps.p.push_back(y);
    ps.p.push_back(z);
}

static void pushU(Particles& ps, float x, float y, float z)
{
    ps.u.push_back(x);
    ps.u.push_back(y);
    ps.u.push_back(z);
}

void pset(Particles& ps)
{
    pushP(ps, 0, 0, 0);
    pushP(ps, 0, 0, 0);
    pushP(ps, 0, 0, 0);
    pushP(ps, 0, 0, 0);

    pushU(ps, 1, 0, 0);
    pushU(ps, 0, 0, 1);
    pushU(ps, 2, 0, 0);
    pushU(ps, 0, 3, 4);
    ps.confirm();
}

TEST(Sph, creation)
{
    Particles ps;
    pset(ps);
    DOUBLES_EQUAL(5.0f, ps.maxVelicty(), 1.0E-3)
}

TEST(Sph, step)
{
    UnitConversion uConv;
    Sph sph(uConv);
    Particles ps;
    WallParticles wps;
    pushP(ps, 0, 0, 0);
    pushU(ps, 1, 0, 0);
    ps.confirm();
    pushP(wps, 100,  0, 0); //Dummy.
    wps.confirm();

    float t = 0.1;
    sph.step(ps, wps, t);
    DOUBLES_EQUAL(sph.DEFAULTG * t * t, ps.p[1], 1.0E-3f);
}

TEST(Sph, density)
{
    UnitConversion uConv;
    Sph sph(uConv);
    Particles ps;
    WallParticles wps;
    for (int x = -5; x < 6; ++x)
    {
        for (int y = -5; y < 6; ++y)
        {
            for (int z = -5; z < 6; ++z)
            {
                pushP(ps, x, y, z);
            }
        }
    }
    ps.confirm();
    pushP(wps, 100,  0, 0); //Dummy.
    wps.confirm();

    sph.step(ps, wps, 1.0f);
    float centerPid = 665;
    //Volume integral of the kernel function is 1.
    DOUBLES_EQUAL(1.0f, ps.d[centerPid], 1.0E-2f);
}

TEST(Sph, density1)
{
    UnitConversion uConv;
    Sph sph(uConv);
    Particles ps;
    WallParticles wps;
    pushP(ps, -2,  0, 0);
    pushP(ps,  0,  0, 0);
    pushP(ps,  2,  0, 0);
    ps.confirm();
    pushP(ps, 100,  0, 0); //Dummy.
    wps.confirm();

    //It does not move particles because the density is less than the rest density.
    sph.step(ps, wps, 1.0f);
    DOUBLES_EQUAL(-2.0f, ps.p[0], 1.0E-3f);
    DOUBLES_EQUAL( 0.0f, ps.p[3], 1.0E-3f);
    DOUBLES_EQUAL( 2.0f, ps.p[6], 1.0E-3f);
}
