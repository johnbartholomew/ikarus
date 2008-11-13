#include "Global.h"
#include "GfxUtil.h"

// ===== Utilities ===========================================================

void renderBlob(const vec3f &col, const vec3d &pos)
{
	glColor3fv(col);
	glBegin(GL_LINES);
	{
		const double size = 0.25;
		const vec3d a(size, 0.0, 0.0);
		const vec3d b(0.0, size, 0.0);
		const vec3d c(0.0, 0.0, size);

		const vec3d v0 = pos - b;
		const vec3d v1 = pos - a;
		const vec3d v2 = pos - c;
		const vec3d v3 = pos + a;
		const vec3d v4 = pos + c;
		const vec3d v5 = pos + b;

		glVertex3dv(v0); glVertex3dv(v1);
		glVertex3dv(v0); glVertex3dv(v2);
		glVertex3dv(v0); glVertex3dv(v3);
		glVertex3dv(v0); glVertex3dv(v4);

		glVertex3dv(v1); glVertex3dv(v2);
		glVertex3dv(v2); glVertex3dv(v3);
		glVertex3dv(v3); glVertex3dv(v4);
		glVertex3dv(v4); glVertex3dv(v1);

		glVertex3dv(v1); glVertex3dv(v5);
		glVertex3dv(v2); glVertex3dv(v5);
		glVertex3dv(v3); glVertex3dv(v5);
		glVertex3dv(v4); glVertex3dv(v5);
	}
	glEnd();
}

void boxPoints(const vec2i &a, const vec2i &b, int cornerRadius, bool line)
{
	if (line)
		glBegin(GL_LINE_LOOP);
	else
		glBegin(GL_POLYGON);

	if (cornerRadius == 0)
	{
		glVertex2i(a.x, a.y);
		glVertex2i(b.x, a.y);
		glVertex2i(b.x, b.y);
		glVertex2i(a.x, b.y);
		
	}
	else
	{
		glVertex2i(a.x+cornerRadius, a.y);
		glVertex2i(b.x-cornerRadius, a.y);
		glVertex2i(b.x             , a.y+cornerRadius);
		glVertex2i(b.x             , b.y-cornerRadius);
		glVertex2i(b.x-cornerRadius, b.y);
		glVertex2i(a.x+cornerRadius, b.y);
		glVertex2i(a.x             , b.y-cornerRadius);
		glVertex2i(a.x             , a.y+cornerRadius);
	}
	
	glEnd();
}

void renderBox(const vec3f &bgCol, const vec3f &borderCol, const recti &rect, int cornerRadius)
{
	vec2i a(rect.topLeft);
	vec2i b(rect.topLeft + rect.size);

	glDisable(GL_TEXTURE_2D);
	glColor3fv(bgCol);
	boxPoints(a, b, cornerRadius, false);
	glColor3fv(borderCol);
	boxPoints(a, b, cornerRadius, true);
}

void arcPoints(const vec3d &centre, const vec3d &normal, const vec3d &zeroDir, double radius, double startAngle, double endAngle)
{
	assert(abs(dot(normal, zeroDir)) < 0.00001);
	vec3d side = cross(normal, zeroDir);

	mat3d orient(
		zeroDir.x, side.x, normal.x,
		zeroDir.y, side.y, normal.y,
		zeroDir.z, side.z, normal.z
	);

	double range = endAngle - startAngle;
	int N = 1 + (int)(range / (M_PI/16.0));
	for (int i = 0; i <= N; ++i)
	{
		double a = startAngle + i*(range/N);
		vec3d v(radius*cos(a), radius*sin(a), 0.0);
		v = orient * v;
		v = centre + v;

		glVertex3d(v.x, v.y, v.z);
	}
}
