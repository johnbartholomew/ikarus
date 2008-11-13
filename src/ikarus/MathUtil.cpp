#include "Global.h"
#include "MathUtil.h"

// ===== Utilities ===========================================================

mat3d calcDirectRotation(const vec3d &tip, const vec3d &target)
{
	double lenSqrTip = dot(tip, tip);
	if (lenSqrTip < 0.001) return mat3d(1.0);
	double lenSqrTarget = dot(target, target);
	if (lenSqrTarget < 0.001) return mat3d(1.0);

	vec3d a = tip * vmath::rsqrt(lenSqrTip);
	vec3d b = target * vmath::rsqrt(lenSqrTarget);

	// sanity check
	assert(abs(dot(a,a) - 1.0) < 0.0001);
	assert(abs(dot(b,b) - 1.0) < 0.0001);

	vec3d axis;

	double dotAB = dot(a, b);
	double angle;
	if (dotAB <= -1.0)
	{
		// angle is 180 degrees; any axis will do...
		angle = M_PI;
		if (abs(dot(a, unitX)) < 0.8)
			axis = normalize(cross(a, unitX));
		else
			axis = normalize(cross(a, unitZ));
	}
	else if (dotAB >= 1.0)
	{
		// angle is zero; no rotation
		return mat3d(1.0);
	}
	else
	{
		angle = std::acos(dotAB);
		axis = cross(a, b);
	}

	// sanity check
	assert(angle >= -M_PI && angle <= M_PI);

	// early-out if the angle is small
	if (angle < 0.001)
		return mat3d(1.0);

	return vmath::rotation_matrix3(angle, axis);
}

mat3d rotationFromAzElTwist(double az, double el, double twist)
{
	vec3d axis(cos(az), 0.0, -sin(az));
	return vmath::rotation_matrix3(el, axis) * vmath::rotation_matrix3(twist, unitY);
}

/*
mat3d rotationFromAzElTwist(double az, double el, double twist)
{
	const double caz = cos(az);
	const double saz = sin(az);
	const double cel = cos(el);
	const double sel = sin(el);

	// elevation * azimuth

	mat3d azM(
		caz, 0.0, saz,
		0.0, 1.0, 0.0,
		-saz, 0.0, caz
	);
	mat3d elM(
		1.0, 0.0, 0.0,
		0.0, cel, -sel,
		0.0, sel, cel
	);

	mat3d M = azM*elM;

#if 0
	mat3d M(
		   caz  ,  0.0,    saz  ,
		-saz*sel,  cel,  sel*caz,
		-saz*cel, -sel,  cel*caz
	);
#endif

	return M * vmath::rotation_matrix3(twist, vec3d(0.0, 1.0, 0.0));
}
*/

void testAzElRotation()
{
	mat3d M;

	vec3d x, y, z;

	const double threshold = 0.000001;

	M = rotationFromAzElTwist(0.0, 0.0, 0.0);
	x = M*unitX; y = M*unitY; z = M*unitZ;
	assert(length_squared(unitX - x) < threshold);
	assert(length_squared(unitY - y) < threshold);
	assert(length_squared(unitZ - z) < threshold);

	M = rotationFromAzElTwist(0.0, M_PI/2.0, 0.0);
	x = M*unitX; y = M*unitY; z = M*unitZ;
	assert(length_squared(unitX - x) < threshold);
	assert(length_squared(unitZ - y) < threshold);
	assert(length_squared(-unitY - z) < threshold);

	M = rotationFromAzElTwist(M_PI, M_PI/2.0, 0.0);
	x = M*unitX; y = M*unitY; z = M*unitZ;
	assert(length_squared(-unitX - x) < threshold);
	assert(length_squared(-unitZ - y) < threshold);
	assert(length_squared(-unitY - z) < threshold);

	M = rotationFromAzElTwist(M_PI, M_PI, 0.0);
	x = M*unitX; y = M*unitY; z = M*unitZ;
	assert(length_squared(-unitX - x) < threshold);
	assert(length_squared(-unitY - y) < threshold);
	assert(length_squared(unitZ - z) < threshold);
}

void directionToAzimuthElevation(const vec3d &dir, double &az, double &el)
{
	const vec3d v = normalize(dir);

	double d = clamp(-1.0, 1.0, v.y);
	el = std::acos(d);

	if (abs(d) == 1.0)
		az = 0.0;
	else
	{
		vec3d vOnPlane(v.x, 0.0, v.z);
		vOnPlane = normalize(vOnPlane);

		d = clamp(-1.0, 1.0, dot(vOnPlane, unitZ));
		az = std::acos(d);
		if (vOnPlane.x < 0.0)
			az = -az;
	}
}
