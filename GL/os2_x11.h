/* os2_x11.h v 0.1 11/03/2000 EK */

#ifndef __os2_x11_h__
#define __os2_x11_h__

#define INCL_GPI
#define INCL_WIN

#include "os2_config.h"

#define __OS2PM__


extern  void  APIENTRY OS2_WMesaInitHab(HAB  proghab,int useDive);

#endif
