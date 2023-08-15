#ifndef HOHE2_wall_H
#define HOHE2_wall_H

#include <math.h>
#include <vector>
#include <iostream>

namespace hohe2
{
struct Wall
{
    float p1[3], p2[3], p3[3];

    void calcNormal(float result[3]);
    float calcS(const float q[3]);
    void calcDeltaPos(const float q[3], float deltaPos[3]);

};

}
#endif
