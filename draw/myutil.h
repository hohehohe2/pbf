#ifndef GLUTIL_H
#define GLUTIL_H

#include <Eigen/Core>
#include <Eigen/Eigen>

extern const Eigen::Vector3f BLACK, RED, GREEN, BLUE, YELLOW, MAGENTA, WHITE, NONE;

void col(const Eigen::Vector3f& col=WHITE);
void point(const Eigen::Vector3f& v1);
void poly(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, const Eigen::Vector3f& v3, const Eigen::Vector3f& v4=NONE, const Eigen::Vector3f& v5=NONE);
void line(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, const Eigen::Vector3f& v3=NONE, const Eigen::Vector3f& v4=NONE);
void translate(const Eigen::Vector3f& t);
void rotate(float angle, const Eigen::Vector3f& axis);
void scale(const Eigen::Vector3f& s);

#endif
