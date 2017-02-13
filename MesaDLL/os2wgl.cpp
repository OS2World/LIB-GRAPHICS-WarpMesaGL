/* $Id: os2wgl.c,v 1.3 1998/09/16 02:46:56 brianp Exp $ */

/*
* File name    : os2wgl.c
* WGL stuff. Added by Oleg Letsinsky, ajl@ultersys.ru
* Some things originated from the 3Dfx WGL functions
* changes EK
*/
/* changes (c) EK */
#define POKA 0


#define USE_MESA 1
#define WARPMESA_USEPMGPI 1
#define WARPMESA_USEDIVE  0

#define GL_GLEXT_PROTOTYPES

#include "GL/gl.h"
#include "GL/glext.h"
//#include <GL/glu.h>

#include <stdio.h>
#include "WarpGL.h"


#include "os2mesadef.h"
#include "GL/os2mesa.h"
#include "mtypes.h"
#include "glapi.h"

#define MAX_MESA_ATTRS 20

struct __pixelformat__
{
    PIXELFORMATDESCRIPTOR      pfd;
    GLboolean doubleBuffered;
};

struct __pixelformat__ pix[] =
{
    /* Double Buffer, alpha */
    {  {       sizeof(PIXELFORMATDESCRIPTOR),  1,
        PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_GENERIC_FORMAT|PFD_DOUBLEBUFFER|PFD_SWAP_COPY,
        PFD_TYPE_RGBA,
        24,    8,      0,      8,      8,      8,      16,     8,      24,
        0,     0,      0,      0,      0,      16,     8,      0,      0,      0,      0,      0,      0 },
        GL_TRUE
    },
    /* Single Buffer, alpha */
    {  {       sizeof(PIXELFORMATDESCRIPTOR),  1,
        PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_GENERIC_FORMAT,
        PFD_TYPE_RGBA,
        24,    8,      0,      8,      8,      8,      16,     8,      24,
        0,     0,      0,      0,      0,      16,     8,      0,      0,      0,      0,      0,      0 },
        GL_FALSE
    },
    /* Double Buffer, no alpha */
    {  {       sizeof(PIXELFORMATDESCRIPTOR),  1,
        PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_GENERIC_FORMAT|PFD_DOUBLEBUFFER|PFD_SWAP_COPY,
        PFD_TYPE_RGBA,
        24,    8,      0,      8,      8,      8,      16,     0,      0,
        0,     0,      0,      0,      0,      16,     8,      0,      0,      0,      0,      0,      0 },
        GL_TRUE
    },
    /* Single Buffer, no alpha */
    {  {       sizeof(PIXELFORMATDESCRIPTOR),  1,
        PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_GENERIC_FORMAT,
        PFD_TYPE_RGBA,
        24,    8,      0,      8,      8,      8,      16,     0,      0,
        0,     0,      0,      0,      0,      16,     8,      0,      0,      0,      0,      0,      0 },
        GL_FALSE
    },
};

int                            qt_ext = sizeof(pix) / sizeof(pix[0]);

#if POKA
struct __pixelformat__ pix[] =
{
    {  {       sizeof(XVisualInfo),  1,
        PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_GENERIC_FORMAT|PFD_DOUBLEBUFFER|PFD_SWAP_COPY,
        PFD_TYPE_RGBA,
        24,    8,      0,      8,      8,      8,      16,     8,      24,
        0,     0,      0,      0,      0,      16,     8,      0,      0,      0,      0,      0,      0 },
        GL_TRUE
    },
    {  {       sizeof(XVisualInfo),  1,
        PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_GENERIC_FORMAT,
        PFD_TYPE_RGBA,
        24,    8,      0,      8,      8,      8,      16,     8,      24,
        0,     0,      0,      0,      0,      16,     8,      0,      0,      0,      0,      0,      0 },
        GL_FALSE
    },
};
#endif

int  qt_pix = sizeof(pix) / sizeof(pix[0]);


typedef struct {
    WMesaContext ctx;
    HDC hdc;
} MesaWglCtx;

#define MESAWGL_CTX_MAX_COUNT 20

static MesaWglCtx wgl_ctx[MESAWGL_CTX_MAX_COUNT];

static unsigned ctx_count = 0;
static unsigned ctx_current = -1;
static unsigned curPFD = 0;

HAB   hab;      /* PM anchor block handle         */
HPS   hpsCurrent;
int   PMuseDive=0;

/* Must be called at initialization */
/* possibly at any thread but yet it is not multy-PM-thread  */
void APIENTRY  OS2_WMesaInitHab(HAB  proghab,int useDive)
{ hab = proghab;
  PMuseDive = useDive;
    /* mask Underflow exeptions for each thread */
    _control87(EM_UNDERFLOW,EM_UNDERFLOW);
//printf("1 useDive=%i\n",useDive);

}



WGLAPI BOOL GLAPIENTRY wglCopyContext(HGLRC hglrcSrc,HGLRC hglrcDst,UINT mask)
{
    return(FALSE);
}


WGLAPI HGLRC GLAPIENTRY wglCreateContext(HDC hdc,HPS hpsCurrent, HAB hab)
{
    HWND               hWnd;
    int i = 0;
    if(!(hWnd = WinWindowFromDC(hdc)))
    {
        SetLastError(0);
        return(NULL);
    }
    if (!ctx_count)
    {
       for(i=0;i<MESAWGL_CTX_MAX_COUNT;i++)
       {
               wgl_ctx[i].ctx = NULL;
               wgl_ctx[i].hdc = NULLHANDLE;
       }
    }
    for( i = 0; i < MESAWGL_CTX_MAX_COUNT; i++ )
    {
        if ( wgl_ctx[i].ctx == NULL )
        {
            wgl_ctx[i].ctx = WMesaCreateContext( hWnd, NULL,
                                       hpsCurrent, hab, GL_TRUE,
                                       pix[curPFD-1].doubleBuffered,PMuseDive );

            if (wgl_ctx[i].ctx == NULL)
                break;
            wgl_ctx[i].hdc = hdc;
            ctx_count++;
            return ((HGLRC)wgl_ctx[i].ctx);
        }
    }
    SetLastError(0);
    return(NULL);
}


WGLAPI BOOL wglDeleteContext(HGLRC hglrc)
{
    int i;
    for ( i = 0; i < MESAWGL_CTX_MAX_COUNT; i++ )
    {
       if ( wgl_ctx[i].ctx == (PWMC) hglrc )
       {
            WMesaMakeCurrent((PWMC)hglrc);
            WMesaDestroyContext();
            wgl_ctx[i].ctx = NULL;
            wgl_ctx[i].hdc = NULLHANDLE;
            ctx_count--;
            return(TRUE);
       }
    }
    SetLastError(0);
    return(FALSE);
}

HGLRC wglCreateLayerContext(HDC hdc,int iLayerPlane)
{
    SetLastError(0);
    return(NULL);
}

HGLRC wglGetCurrentContext(VOID)
{
   if (ctx_current < 0)
      return 0;
   else
      return wgl_ctx[ctx_current].ctx;
}

HDC wglGetCurrentDC(VOID)
{
   if (ctx_current < 0)
      return 0;
   else
      return wgl_ctx[ctx_current].hdc;
}

BOOL wglMakeCurrent(HDC hdc,HGLRC hglrc)
{
    int i;

    /* new code suggested by Andy Sy */
    if (!hdc || !hglrc) {
       WMesaMakeCurrent(NULL);
       ctx_current = -1;
       return TRUE;
    }

    for ( i = 0; i < MESAWGL_CTX_MAX_COUNT; i++ )
    {
        if ( wgl_ctx[i].ctx == hglrc )
        {
            wgl_ctx[i].hdc = hdc;
            WMesaMakeCurrent( (WMesaContext) hglrc );
            ctx_current = i;
            return TRUE;
        }
    }
    return FALSE;
}


BOOL  wglShareLists(HGLRC hglrc1,HGLRC hglrc2)
{
    return(TRUE);
}

BOOL  wglUseFontBitmapsA(HDC hdc,int first,int count,int listBase)
{
    return(FALSE);
}

BOOL  wglUseFontBitmapsW(HDC hdc,int first,int count,int listBase)
{
    return(FALSE);
}

#if POKA
BOOL  wglUseFontOutlinesA(HDC hdc,int first,int count,
                                  int listBase,float deviation,
                                  float extrusion,int format,
                                  LPGLYPHMETRICSFLOAT lpgmf)
{
    SetLastError(0);
    return(FALSE);
}

 BOOL  wglUseFontOutlinesW(HDC hdc,int first,int count,
                                  int listBase,float deviation,
                                  float extrusion,int format,
                                  LPGLYPHMETRICSFLOAT lpgmf)
{
    SetLastError(0);
    return(FALSE);
}

 BOOL  wglDescribeLayerPlane(HDC hdc,int iPixelFormat,
                                    int iLayerPlane,UINT nBytes,
                                    LPLAYERPLANEDESCRIPTOR plpd)
{
    SetLastError(0);
    return(FALSE);
}

 int  wglSetLayerPaletteEntries(HDC hdc,int iLayerPlane,
                                       int iStart,int cEntries,
                                       CONST COLORREF *pcr)
{
    SetLastError(0);
    return(0);
}

 int  wglGetLayerPaletteEntries(HDC hdc,int iLayerPlane,
                                       int iStart,int cEntries,
                                       COLORREF *pcr)
{
    SetLastError(0);
    return(0);
}

#endif /* POKA */

 BOOL  wglRealizeLayerPalette(HDC hdc,int iLayerPlane,BOOL bRealize)
{
    SetLastError(0);
    return(FALSE);
}

 BOOL  wglSwapLayerBuffers(HDC hdc,UINT fuPlanes)
{
    if( !hdc )
    {
        WMesaSwapBuffers();
        return(TRUE);
    }
    SetLastError(0);
    return(FALSE);
}


 int  wglChoosePixelFormat(HDC hdc,
                                   PIXELFORMATDESCRIPTOR *ppfd)
{
    int                i,best = -1,bestdelta = 0x7FFFFFFF,delta,qt_valid_pix;

    qt_valid_pix = qt_pix;
    if(ppfd->nSize != sizeof(PIXELFORMATDESCRIPTOR) || ppfd->nVersion != 1)
    {
        SetLastError(0);
        return(0);
    }
    for(i = 0;i < qt_valid_pix;i++)
    {
        delta = 0;
        if(
            (ppfd->dwFlags & PFD_DRAW_TO_WINDOW) &&
            !(pix[i].pfd.dwFlags & PFD_DRAW_TO_WINDOW))
            continue;
        if(
            (ppfd->dwFlags & PFD_DRAW_TO_BITMAP) &&
            !(pix[i].pfd.dwFlags & PFD_DRAW_TO_BITMAP))
            continue;
        if(
            (ppfd->dwFlags & PFD_SUPPORT_GDI) &&
            !(pix[i].pfd.dwFlags & PFD_SUPPORT_GDI))
            continue;
        if(
            (ppfd->dwFlags & PFD_SUPPORT_OPENGL) &&
            !(pix[i].pfd.dwFlags & PFD_SUPPORT_OPENGL))
            continue;
        if(
            !(ppfd->dwFlags & PFD_DOUBLEBUFFER_DONTCARE) &&
            ((ppfd->dwFlags & PFD_DOUBLEBUFFER) != (pix[i].pfd.dwFlags & PFD_DOUBLEBUFFER)))
            continue;
        if(
            !(ppfd->dwFlags & PFD_STEREO_DONTCARE) &&
            ((ppfd->dwFlags & PFD_STEREO) != (pix[i].pfd.dwFlags & PFD_STEREO)))
            continue;
        if(ppfd->iPixelType != pix[i].pfd.iPixelType)
            delta++;
        if(delta < bestdelta)
        {
            best = i + 1;
            bestdelta = delta;
            if(bestdelta == 0)
                break;
        }
    }
    if(best == -1)
    {
        SetLastError(0);
        return(0);
    }
    return(best);
}


XVisualInfo *wglDescribePixelFormat(int iPixelFormat)
{
    if(iPixelFormat < 1 || iPixelFormat > qt_pix)
    {
        SetLastError(0);
        return(0);
    }
    return  &(pix[iPixelFormat - 1].pfd);
}

#if POKA

/*
* GetProcAddress - return the address of an appropriate extension
*/
 PROC  wglGetProcAddress(LPCSTR lpszProc)
{
    int                i;
    for(i = 0;i < qt_ext;i++)
        if(!strcmp(lpszProc,ext[i].name))
            return(ext[i].proc);

        SetLastError(0);
        return(NULL);
}

#endif /* POKA */
 int  wglGetPixelFormat(HDC hdc)
{
    if(curPFD == 0)
    {
        SetLastError(0);
        return(0);
    }
    return(curPFD);
}

/* I don't understand all this suxx PixelFormat manipulation */
int evglSetPixelFormat(int iPixelFormat)
{
   if(iPixelFormat == 1  || iPixelFormat == 2)
         curPFD = iPixelFormat;
   else
         curPFD = 1;
   return 0;
}

 BOOL  wglSetPixelFormat(HDC hdc,int iPixelFormat,
                                PIXELFORMATDESCRIPTOR *ppfd)
{
    int                qt_valid_pix;

    qt_valid_pix = qt_pix;
#if POKA == 200
    if(iPixelFormat < 1 || iPixelFormat > qt_valid_pix || ppfd->nSize != sizeof(PIXELFORMATDESCRIPTOR))
    {
        SetLastError(0);
        return(FALSE);
    }
#endif /*  POKA == 200 */
    curPFD = iPixelFormat;
    return(TRUE);
}



BOOL  wglSwapBuffers(void)
{
   if (ctx_current < 0)
      return FALSE;

   if(wgl_ctx[ctx_current].ctx == NULL) {
      SetLastError(0);
      return(FALSE);
   }
   WMesaSwapBuffers();
   return(TRUE);
}


/*************************************/
static int LastError=0;

int SetLastError(int nerr)
{  LastError = nerr;
   return 0;
}
int GetLastError(void)
{  return LastError;
}

