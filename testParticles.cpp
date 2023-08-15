#include <iostream>
#include <sstream>
#include <CppUnitLite/TestHarness.h>
#include "particles.h"

using namespace hohe2;

static void pushP(Particles& ps, float x, float y, float z)
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

void set(Particles& ps)
{
    pushP(ps, 0, 0, 0);
    pushP(ps, 0, 0, 0);
    pushP(ps, 0, 0, 0);
    pushP(ps, 0, 0, 0);

    pushU(ps, 1, 0, 0);
    pushU(ps, 0, 0, 1);
    pushU(ps, 2, 0, 0);
    pushU(ps, 0, 3, 4);
}

TEST(Particles, maxVelicty)
{
    Particles ps;
    set(ps);
    DOUBLES_EQUAL(5.0, ps.maxVelicty(), 1.0E-3);
}

TEST(Particles, add)
{
    Particles ps;
    Particles psadd;
    set(psadd);
    ps.addParticles(psadd);
    DOUBLES_EQUAL(5.0, ps.maxVelicty(), 1.0E-3);
}

TEST(Particles, clear)
{
    Particles ps;
    set(ps);
    ps.clear();
    DOUBLES_EQUAL(1.0E-16, ps.maxVelicty(), 1.0E-3);
}

TEST(Particles, ostream)
{
    Particles ps;
    pushP(ps, 0.1, 0.2, 0.3);
    pushU(ps, 1.1, 1.2, 1.3);
    ps.d.push_back(111.1);

    std::stringstream ss;
    ss << ps;
    std::string s = "1\n0.100000 0.200000 0.300000 1.100000 1.200000 1.300000 111.099998\n";
    CHECK_EQUAL(s, ss.str());
}

TEST(Particles, istream)
{
    std::string s = "1\n0.1 0.2 0.3 1.1 1.2 1.3 111.1\n";
    std::stringstream ss(s);

    Particles ps;
    ss >> ps;
    DOUBLES_EQUAL(0.1, ps.p[0], 1.0E-3);
    DOUBLES_EQUAL(0.2, ps.p[1], 1.0E-3);
    DOUBLES_EQUAL(0.3, ps.p[2], 1.0E-3);
    DOUBLES_EQUAL(1.1, ps.u[0], 1.0E-3);
    DOUBLES_EQUAL(1.2, ps.u[1], 1.0E-3);
    DOUBLES_EQUAL(1.3, ps.u[2], 1.0E-3);
    DOUBLES_EQUAL(111.1, ps.d[0], 1.0E-3);
}

TEST(Particles, ostream2)
{
    Particles ps;
    pushP(ps, 0.1, 0.2, 0.3);
    pushP(ps, 0.4, 0.5, 0.6);
    pushU(ps, 1.1, 1.2, 1.3);
    pushU(ps, 1.4, 1.5, 1.6);
    ps.d.push_back(111.1);
    ps.d.push_back(111.2);

    std::stringstream ss;
    ss << ps;
    std::string s = "2\n0.100000 0.200000 0.300000 1.100000 1.200000 1.300000 111.099998\n0.400000 0.500000 0.600000 1.400000 1.500000 1.600000 111.199997\n";
    CHECK_EQUAL(s, ss.str());
}

TEST(Particles, istream2)
{
    std::string s = "2\n0.1 0.2 0.3 1.1 1.2 1.3 111.1\n0.4 0.5 0.6 1.4 1.5 1.6 111.2\n";
    std::stringstream ss(s);

    Particles ps;
    ss >> ps;
    DOUBLES_EQUAL(0.1, ps.p[0], 1.0E-3);
    DOUBLES_EQUAL(0.2, ps.p[1], 1.0E-3);
    DOUBLES_EQUAL(0.3, ps.p[2], 1.0E-3);
    DOUBLES_EQUAL(1.1, ps.u[0], 1.0E-3);
    DOUBLES_EQUAL(1.2, ps.u[1], 1.0E-3);
    DOUBLES_EQUAL(1.3, ps.u[2], 1.0E-3);
    DOUBLES_EQUAL(111.1, ps.d[0], 1.0E-3);

    DOUBLES_EQUAL(0.4, ps.p[3], 1.0E-3);
    DOUBLES_EQUAL(0.5, ps.p[4], 1.0E-3);
    DOUBLES_EQUAL(0.6, ps.p[5], 1.0E-3);
    DOUBLES_EQUAL(1.4, ps.u[3], 1.0E-3);
    DOUBLES_EQUAL(1.5, ps.u[4], 1.0E-3);
    DOUBLES_EQUAL(1.6, ps.u[5], 1.0E-3);
    DOUBLES_EQUAL(111.2, ps.d[1], 1.0E-3);
}

TEST(Particles, istream3)
{
    std::string s;
    s = "1\n0.1 0.2 0.3 1.1 1.2 1.3 111.1\n";
    std::stringstream ss1(s);
    s = "1\n0.4 0.5 0.6 1.4 1.5 1.6 111.2\n";
    std::stringstream ss2(s);

    Particles ps;
    ss1 >> ps;
    ss2 >> ps;

    DOUBLES_EQUAL(0.1, ps.p[0], 1.0E-3);
    DOUBLES_EQUAL(0.2, ps.p[1], 1.0E-3);
    DOUBLES_EQUAL(0.3, ps.p[2], 1.0E-3);
    DOUBLES_EQUAL(1.1, ps.u[0], 1.0E-3);
    DOUBLES_EQUAL(1.2, ps.u[1], 1.0E-3);
    DOUBLES_EQUAL(1.3, ps.u[2], 1.0E-3);
    DOUBLES_EQUAL(111.1, ps.d[0], 1.0E-3);

    DOUBLES_EQUAL(0.4, ps.p[3], 1.0E-3);
    DOUBLES_EQUAL(0.5, ps.p[4], 1.0E-3);
    DOUBLES_EQUAL(0.6, ps.p[5], 1.0E-3);
    DOUBLES_EQUAL(1.4, ps.u[3], 1.0E-3);
    DOUBLES_EQUAL(1.5, ps.u[4], 1.0E-3);
    DOUBLES_EQUAL(1.6, ps.u[5], 1.0E-3);
    DOUBLES_EQUAL(111.2, ps.d[1], 1.0E-3);
}

TEST(Particles, confirm)
{
    Particles ps;
    CHECK_EQUAL(true, ps.confirm(false));
    CHECK_EQUAL(true, ps.confirm(true));
    pushP(ps, 0, 0, 0);
    CHECK_EQUAL(false, ps.confirm(false));
    CHECK_EQUAL(false, ps.confirm(false));
    CHECK_EQUAL(true, ps.confirm(true));
    CHECK_EQUAL(true, ps.confirm(true));
}
