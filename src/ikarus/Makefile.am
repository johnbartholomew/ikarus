bin_PROGRAMS = ikarus
noinst_HEADERS = \
	Camera.h \
	FileUtil.h \
	Font.h \
	GfxUtil.h \
	Global.h \
	IkSolver.h \
	MathUtil.h \
	murmurhash.h \
	OrbGui.h \
	OrbInput.h \
	OrbWidgetID.h \
	Pose.h \
	refvector.h \
	scopedenum.h \
	SkeletonDisplay.h \
	Skeleton.h \
	smartptr.h \
	Texture.h \
	VertexBuffer.h \
	OrbGLFW.h \
	vmath.h

ikarus_SOURCES = \
	Camera.cpp \
	Font.cpp \
	GfxUtil.cpp \
	Ikarus.cpp \
	IkSolver.cpp \
	MathUtil.cpp \
	OrbGui.cpp \
	OrbInput.cpp \
	PCH.cpp \
	Pose.cpp \
	Skeleton.cpp \
	SkeletonDisplay.cpp \
	Texture.cpp \
	VertexBuffer.cpp \
	OrbGLFW.cpp

ikarus_CFLAGS = $(ALL_CFLAGS)
ikarus_LDADD = $(ALL_LIBS)

# Disabled sources:
dist_noinst_DATA = \
	OrbSDL.h \
	OrbSDL.cpp

# Win32 sources: included in the distribution tarball for completeness

dist_noinst_DATA += \
	win32/resources.h \
	win32/OrbWin32.h \
	win32/OrbWin32.cpp \
	win32/Win32OpenGLContext.h \
	win32/Win32OpenGLContext.cpp \
	win32/Win32Error.h \
	win32/Win32Error.cpp \
	win32/Win32Window.h \
	win32/Win32Window.cpp
