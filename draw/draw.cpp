#include <fstream>
#include "draw.h"
#include "myutil.h"

using namespace hohe2;

Particles ps;
static std::ifstream ifs("../result.txt");
static float t;

void initialize()
{
    step();
}

void step()
{
    ps.clear();
    ifs >> t >> ps;    
    std::cout << t << " " << std::endl;
}

void draw()
{
    int size = ps.getNumParticles();
    for (float pid = 0; pid < size; ++pid)
    {
        float x = ps.p[pid * 3 + 0];
        float y = ps.p[pid * 3 + 1];
        float z = ps.p[pid * 3 + 2];
        int isWall = ps.isWall[pid];
        if (! isWall)
        {
            col(BLUE);
            point(Eigen::Vector3f(x, y, z));
        }
    }
}
