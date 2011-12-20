dnl Process this file with autoconf to create configure.

AC_PREREQ(2.59)

AC_INIT([ikarus], [0.00], [jpa.bartholomew@gmail.com])

AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2 dist-xz dist-zip])
AM_SILENT_RULES([yes])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX
AC_C_CONST
AC_PROG_RANLIB
#AC_PROG_LIBTOOL

AC_CHECK_LIB([GL], [glGetString], [GL_LIBS="-lGL"], [AC_MSG_ERROR([OpenGL is required])])
AC_CHECK_LIB([SOIL], [SOIL_load_image], [SOIL_LIBS="-lSOIL"], [AC_MSG_ERROR([SOIL is required])], [-lGL -lm])
PKG_CHECK_MODULES([GLEW], [glew >= 1.5], [], [AC_MSG_ERROR([GLEW is required])])
PKG_CHECK_MODULES([GLFW], [libglfw >= 2.7], [], [AC_MSG_ERROR([GLFW is required])])

ALL_CFLAGS="${GLFW_CFLAGS} ${GLEW_CFLAGS}"
ALL_CXXFLAGS="$ALL_CFLAGS"
ALL_LIBS="${GLFW_LIBS} ${SOIL_LIBS} ${GLEW_LIBS} ${GL_LIBS}"
AC_SUBST(ALL_CFLAGS)
AC_SUBST(ALL_CXXFLAGS)
AC_SUBST(ALL_LIBS)

AC_CONFIG_FILES([
	Makefile 
	src/Makefile
	src/ikarus/Makefile
])
AC_OUTPUT
