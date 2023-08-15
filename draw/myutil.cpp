#include "myutil.h"
#include <GL/glut.h>


const Eigen::Vector3f BLACK(0, 0, 0),
    RED(1, 0, 0),
    GREEN(0, 1, 0),
    BLUE(0, 0, 1),
    MAGENTA(1, 0, 1),
    YELLOW(1, 1, 0),
    WHITE(1, 1, 1),
    NONE(-100000, -1004343, 432432);

void col(const Eigen::Vector3f& col)
{
    glColor3fv(col.data());
}

void point(const Eigen::Vector3f& v1)
{
    glPointSize(5);
    glBegin( GL_POINTS );
    glVertex3fv(v1.data());
    glEnd();
}

void poly(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, const Eigen::Vector3f& v3, const Eigen::Vector3f& v4, const Eigen::Vector3f& v5)
{
    glBegin( GL_POLYGON );
    glVertex3fv(v1.data());
    glVertex3fv(v2.data());
    glVertex3fv(v3.data());
    if (v4 != NONE)
    {
        glVertex3fv(v4.data());
    }
    if (v5 != NONE)
    {
        glVertex3fv(v5.data());
    }
    glEnd();
}

void line(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, const Eigen::Vector3f& v3, const Eigen::Vector3f& v4)
{
    glBegin( GL_LINE_STRIP );
    glVertex3fv(v1.data());
    glVertex3fv(v2.data());
    if (v3 != NONE)
    {
        glVertex3fv(v3.data());
    }
    if (v4 != NONE)
    {
        glVertex3fv(v4.data());
    }
    glEnd();
}

void translate(const Eigen::Vector3f& t)
{
    glTranslatef(t[0], t[1], t[2]);
}

void rotate(float angle, const Eigen::Vector3f& axis)
{
    glRotatef(angle, axis[0], axis[1], axis[2]);
}

void scale(const Eigen::Vector3f& s)
{
    glScalef(s[0], s[1], s[2]);
}
