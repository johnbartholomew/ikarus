#ifndef GLOBAL_H
#define GLOBAL_H

// global standard libraries

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

// global non-standard libraries

#define _WIN32_WINNT 0x0501
#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/glfw.h>

// global internal headers

#include "vector_math.h"

typedef vmath::vec3<double> vec3d;
typedef vmath::vec2<double> vec2d;
typedef vmath::vec4<double> vec4d;

typedef vmath::mat3<double> mat3d;
typedef vmath::mat2<double> mat2d;
typedef vmath::mat4<double> mat4d;

typedef vmath::quat<double> quatd;

#endif
