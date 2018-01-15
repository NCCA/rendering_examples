#ifndef GLINCLUDE_H
#define GLINCLUDE_H

// Includes for OpenGL
#define GL_GLEXT_PROTOTYPES
#if ( (defined(__MACH__)) && (defined(__APPLE__)) )
#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#elif (defined(WIN32))
#include <GL/glew.h>
//#include <GL/glu.h>
#include <GL/gl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
//#include <GL/glu.h>
#include <GL/glext.h>
#endif

#endif // GLINCLUDE_H

