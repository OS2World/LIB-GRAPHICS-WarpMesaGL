// BASIS.H

// 0 - work with traditional IBM OpenGL
// 1 - work with WarpMesa 
#define USE_MESA 1
#define WARPMESA_USEPMGPI 1
#define WARPMESA_USEDIVE  0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#ifdef USE_MESA
  #if USE_MESA
     #include "GL\gl.h"
     #include "GL\glut.h"
     #include "GL_TEST.H"
  #endif

#else
  #include <GL\gl.h>
  #include <GL\glut.h>
#endif


