/* os2_config.h v 0.0 14/12/2002 EK */

#ifndef __os2_mesa_config_h__
#define __os2_mesa_config_h__

#define INCL_DOSPROCESS
   #include <os2.h>

  #define WINGDIAPI 
  #define WINAPI
#define CALLBACK

#define CDECL __cdecl

/* Warning! use only if you know what are you doing */
/* Optlink is VAC specific and is a bit faster,     
   but it may be not compatble with user compiler
*/
#define USE_OPTLINK

#if defined(USE_OPTLINK)
 #define GLAPIENTRY    _Optlink
 #define GLCALLBACK    _Optlink
 #define GLUTAPIENTRY  _Optlink
 #define GLUTCALLBACK  _Optlink
#else
 #define GLAPIENTRY    APIENTRY
 #define GLCALLBACK    APIENTRY
 #define GLUTAPIENTRY  APIENTRY
 #define GLUTCALLBACK  APIENTRY
#endif

#define GLCALLBACKP *
#define GLCALLBACKPCAST *
#define GLWINAPI
#define GLWINAPIV

#define GLAPI extern


#endif
   //__os2_mesa_config_h__