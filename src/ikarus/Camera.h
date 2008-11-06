#ifndef CAMERA_H
#define CAMERA_H

class OrbInput;

class Camera
{
public:
	virtual void update(const OrbInput &input, const recti &bounds) = 0;
	virtual void renderUI(const recti &bounds) const = 0;
	virtual mat4d getProjection(const recti &bounds) const = 0;
	virtual mat4d getModelView() const = 0;
};

class CameraOrtho : public Camera
{
public:
	CameraOrtho(int axis);

	virtual void update(const OrbInput &input, const recti &bounds);
	virtual void renderUI(const recti &bounds) const;
	virtual mat4d getProjection(const recti &bounds) const;
	virtual mat4d getModelView() const;
	
private:
	double scale;
	int axis;
};

class CameraAzimuthElevation : public Camera
{
public:
	CameraAzimuthElevation();

	virtual void update(const OrbInput &input, const recti &bounds);
	virtual mat4d getProjection(const recti &bounds) const;
	virtual mat4d getModelView() const;
	virtual void renderUI(const recti &bounds) const;

private:
	bool dragging;
	double cameraDist;

	vec3d pt0, pt1;
	double az0, el0;
	double az, el;

	void startDrag(const vec2i &screenCentre, double screenRadius, const vec2i &pos);
	void updateDrag(const vec2i &screenCentre, double screenRadius, const vec2i &pos);

	// constrains a point on the unit sphere to the given axis (0 = x, 1 = y, 2 = z)
	vec3d constrainToAxis(vec3d pt, int axis) const;
	vec3d screenToSphere(const vec2i &screenCentre, double screenRadius, vec2i pos) const;
	vec2i sphereToScreen(const vec2i &screenCentre, const double screenRadius, const vec3d &pt) const;
};

#endif
