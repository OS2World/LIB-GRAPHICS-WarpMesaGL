/*     File name       :       os2mesadef.h
 */


#ifndef DDMESADEF_H
#define DDMESADEF_H

#include <dive.h>

#include "GL\gl.h"
#include "context.h"

#define REDBITS                0x03
#define REDSHIFT       0x00
#define GREENBITS      0x03
#define GREENSHIFT     0x03
#define BLUEBITS       0x02
#define BLUESHIFT      0x06

//typedef struct _dibSection{
//       HDC             hDC;
//       void *  hFileMap;
//       BOOL    fFlushed;
//       void *  base;
//}WMDIBSECTION, *PWMDIBSECTION;

typedef struct _memoryBuffer
{
  PBYTE  pBmpBuffer;
  int LbmpBuff;
  int bNx,bNy;
  int BytesPerBmpPixel;
  BITMAPINFO2    bmi_mem;
 HDC hdcMem;
 HPS hpsMem;

} MEMBUF, *PMEMBUF;


struct VideoDevConfigCaps
{ int  sts;
  LONG Caps[CAPS_DEVICE_POLYSET_POINTS];
};


typedef struct wmesa_context{
    GLcontext *gl_ctx;         /* The core GL/Mesa context */
    GLvisual *gl_visual;               /* Describes the buffers */
    GLframebuffer *gl_buffer;  /* Depth, stencil, accum, etc buffers */
    HAB             hab; 
    HWND            Window;
    HDC             hDC;
    HPAL            hPalette;
    HPAL            hOldPalette;

// Dive variables
    int             useDive;
    HDIVE           hDive;
    DIVE_CAPS       DiveCaps;          //    = {0};
    FOURCC          fccFormats[100];   //    = {0};
    BOOL            bDiveHaveBuffer;   //    = FALSE;
    ULONG           ulDiveBufferNumber;// = 0;
    int             xDiveScr,yDiveScr; // left low corner
//??    HPEN                hPen;
//??    HPEN                hOldPen;
//??    HCURSOR             hOldCursor;
//??    COLORREF            crColor;

    // 3D projection stuff

    RECTL                drawRect;
    UINT                uiDIBoffset;
    // OpenGL stuff

    HPAL            hGLPalette;
       GLuint                          width;
       GLuint                          height;
       GLuint                          ScanWidth;
       GLboolean                       db_flag;        // * double buffered?
       GLboolean                       rgb_flag;       // * RGB mode?
       GLboolean                       dither_flag;    // * use dither when 256 color mode for RGB?
       GLuint                          depth;          // * bits per pixel (1, 8, 24, etc)
       ULONG                           pixel;  // current color index or RGBA pixel value
       ULONG                           clearpixel; // * pixel for clearing the color buffers
       PBYTE                           ScreenMem; // WinG memory
       BITMAPINFO                      *IndexFormat;
       HPS                         hps;  // (current ?) presentation space handle
       HPAL                        hPal; // Current Palette
       HPAL                        hPalHalfTone;

//       WMDIBSECTION            dib;
//       HBITMAP             hbmDIB;

    MEMBUF               memb;
    BITMAPINFOHEADER2    bmi;
    HBITMAP              hbm;

    HBITMAP             hOldBitmap;
       HBITMAP                         Old_Compat_BM;
       HBITMAP                         Compat_BM;            // Bitmap for double buffering
    PBYTE               pbPixels;
    int                 nColors;
    int                 nColorBits;
       int                                     pixelformat;

        RECTL                                  rectOffScreen;
       RECTL                                   rectSurface;
       HWND                                    hwnd;
       int                                   pitch;
       PBYTE                                   addrOffScreen;

}  *PWMC;




struct DISPLAY_OPTIONS {
       int  stereo;
       int  fullScreen;
       int      mode;
       int      bpp;
};


#define PAGE_FILE              0xffffffff



#endif
