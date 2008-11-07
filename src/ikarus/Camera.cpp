#include "Global.h"
#include "Camera.h"
#include "OrbInput.h"

// ===== CameraOrtho =========================================================

CameraOrtho::CameraOrtho(int axis)
:	axis(axis),
	scale(GridWidth/2.0) // nb: if you change this change update()
{
}

void CameraOrtho::update(const OrbInput &input, const recti &bounds)
{
	int wheel = input.getMouseWheelPos();
	// nb: if you change this change the CameraOrtho initializer
	scale = (GridWidth/2.0) * std::pow(CameraDistWheelScale, -wheel);
}

void CameraOrtho::renderUI(const recti &bounds) const
{
}

mat4d CameraOrtho::getProjection(const recti &bounds) const
{
	double aspect = (double)bounds.size.x / (double)bounds.size.y;
	double sx = (double)bounds.size.x / 2.0;
	double sy = (double)bounds.size.y / 2.0;
	double x = (double)bounds.topLeft.x + sx;
	double y = (double)bounds.topLeft.y + sy;
	return mat4d(
		sx , 0.0, 0.0,   x,
		0.0, -sy, 0.0,   y,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	) * vmath::ortho_matrix(-aspect, aspect, -1.0, 1.0, 0.1, 100.0);
}

mat4d CameraOrtho::getModelView() const
{
	mat4d m;
	if (axis == 0)
	{
		m = mat4d(
			0.0, 0.0, 1.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		);
	}
	else if (axis == 1)
	{
		m = mat4d(
			1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, -1.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		);
	}
	else if (axis == 2)
	{
		m = mat4d(
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		);
	}

	return vmath::scaling_matrix(1.0/scale, 1.0/scale, 1.0) * m * vmath::translation_matrix(0.0, -GridWidth/4.0, 0.0);
}

// ===== CameraAzimuthElevation ==============================================

CameraAzimuthElevation::CameraAzimuthElevation()
:	dragging(false),
	cameraDist(CameraDistance),
	az(0.0), el(0.0), az0(0.0), el0(0.0)
{
}

void CameraAzimuthElevation::update(const OrbInput &input, const recti &bounds)
{
	const vec2i screenCentre = bounds.topLeft + vec2i(bounds.size.x/2, bounds.size.y/2);
	const double screenRadius = std::min(bounds.size.x, bounds.size.y) / 2.0;

	int wheel = input.getMouseWheelPos();
	cameraDist = CameraDistance * std::pow(CameraDistWheelScale, -wheel);

	vec2i mousePos = input.getMousePos();
	if (input.isMouseDown(MouseButton::Right))
	{
		if (!dragging)
		{
			vec2i clickPos = input.getMouseClickPos(MouseButton::Right);
			if (bounds.contains(clickPos))
			{
				dragging = true;
				startDrag(screenCentre, screenRadius, clickPos);
			}
		}
		else
			updateDrag(screenCentre, screenRadius, mousePos);
	}
	else
	{
		if (dragging)
		{
			updateDrag(screenCentre, screenRadius, mousePos);
			dragging = false;
		}
	}
}

mat4d CameraAzimuthElevation::getProjection(const recti &bounds) const
{
	double aspect = (double)bounds.size.x / (double)bounds.size.y;
	double sx = (double)bounds.size.x / 2.0;
	double sy = (double)bounds.size.y / 2.0;
	double x = (double)bounds.topLeft.x + sx;
	double y = (double)bounds.topLeft.y + sy;
	return mat4d(
		sx , 0.0, 0.0,   x,
		0.0, -sy, 0.0,   y,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	) * vmath::perspective_matrix(FoV, aspect, zNear, zFar);
}

mat4d CameraAzimuthElevation::getModelView() const
{
	return vmath::translation_matrix(0.0, -GridWidth/4.0, -cameraDist) * vmath::azimuth_elevation_matrix(az, el);
}

void CameraAzimuthElevation::renderUI(const recti &bounds) const
{
	const vec2i screenCentre = bounds.topLeft + vec2i(bounds.size.x/2, bounds.size.y/2);
	const double screenRadius = std::min(bounds.size.x, bounds.size.y) / 2.0;

	if (dragging)
	{
		glDisable(GL_TEXTURE_2D);

		vec2i pos0 = sphereToScreen(screenCentre, screenRadius, pt0);
		vec2i pos1 = sphereToScreen(screenCentre, screenRadius, pt1);

		glPointSize(5.0);
		glBegin(GL_POINTS);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2i(pos0.x, pos0.y);

		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex2i(pos1.x, pos1.y);
		glEnd();

		glColor3f(0.7f, 0.7f, 0.7f);
		glBegin(GL_LINE_STRIP);
		const int N = 30;
		for (int i = 0; i <= N; ++i)
		{
			double a = (double)i / (double)N;
			vec2i p = sphereToScreen(screenCentre, screenRadius, slerp(quatd(pt0, 0.0), quatd(pt1, 0.0), a).v);
			glVertex2i(p.x, p.y);
		}
		glEnd();
	}
}

void CameraAzimuthElevation::startDrag(const vec2i &screenCentre, double screenRadius, const vec2i &pos)
{
	pt0 = pt1 = screenToSphere(screenCentre, screenRadius, pos);
	az0 = az;
	el0 = el;
}

void CameraAzimuthElevation::updateDrag(const vec2i &screenCentre, double screenRadius, const vec2i &pos)
{
	pt1 = screenToSphere(screenCentre, screenRadius, pos);

	vec3d a, b;
	double dotAB;

	a = constrainToAxis(pt0, 1);
	b = constrainToAxis(pt1, 1);
	dotAB = dot(a, b);
	if (dotAB > 1.0) dotAB = 1.0;
	if (dotAB < -1.0) dotAB = -1.0;
	double deltaAz = std::acos(dot(a, b));
	if (b.x < a.x)
		deltaAz *= -1.0;
	
	a = constrainToAxis(pt0, 0);
	b = constrainToAxis(pt1, 0);
	dotAB = dot(a, b);
	if (dotAB > 1.0) dotAB = 1.0;
	if (dotAB < -1.0) dotAB = -1.0;
	double deltaEl = std::acos(dot(a, b));
	if (b.y < a.y)
		deltaEl *= -1.0;

	az = az0 + deltaAz;
	el = el0 + deltaEl;
}

// constrains a point on the unit sphere to the given axis (0 = x, 1 = y, 2 = z)
vec3d CameraAzimuthElevation::constrainToAxis(vec3d pt, int axis) const
{
	pt[axis] = 0.0;
	if (dot(pt, pt) <= 0.00001)
	{
		// FIXME: vector is nearly zero, what should we do?!
		if (axis == 2)
			return vec3d(0.0, 1.0, 0.0);
		else
			return vec3d(0.0, 0.0, 1.0);
	}
	else
		return normalize(pt);
}

vec3d CameraAzimuthElevation::screenToSphere(const vec2i &screenCentre, double screenRadius, vec2i pos) const
{
	pos -= screenCentre;
	pos.y *= -1;

	assert(screenRadius > 0.0);
	vec3d v(pos.x / screenRadius, pos.y / screenRadius, 0.0);

	double r = v.x*v.x + v.y*v.y;
	if (r > 1.0) // normalize v
		v *= vmath::rsqrt(r);
	else
		v.z = std::sqrt(1.0 - r);

	return v;
}

vec2i CameraAzimuthElevation::sphereToScreen(const vec2i &screenCentre, const double screenRadius, const vec3d &pt) const
{
	assert(screenRadius > 0.0);
	vec2i pos((int)(pt.x*screenRadius), (int)(pt.y*screenRadius));
	pos.y *= -1;
	pos += screenCentre;
	return pos;
}
