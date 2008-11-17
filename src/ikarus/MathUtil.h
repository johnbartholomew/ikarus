#ifndef MATH_UTIL_H
#define MATH_UTIL_H

mat3d calcDirectRotation(const vec3d &tip, const vec3d &target);
mat3d rotationFromAzElTwist(double az, double el, double twist);
void directionToAzimuthElevation(const vec3d &dir, double &az, double &el);
void rotationToAzimuthElevationTwist(const mat3d &rot, vec3d &dir, double &az, double &el, double &twist);

void testAzElRotation();

#endif
