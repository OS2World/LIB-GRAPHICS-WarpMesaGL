/* os2mesa.c,v 0.1 27/02/2000 EK */
#include <stdio.h>
#include <stdlib.h>
#include "WarpGL.h"
#include "GL/os2mesa.h"
#include "os2mesadef.h"
#include "colors.h"
#include "context.h"
#include "colormac.h"
#include "extensions.h"
#include "matrix.h"
#include "texformat.h"
#include "texstore.h"
#include "array_cache/acache.h"
#include "swrast_setup/swrast_setup.h"
#include "swrast/s_alphabuf.h"
#include "tnl/tnl.h"
#include "tnl/t_context.h"
#include "tnl/t_pipeline.h"


   #include "glutint.h"
   /* ???? */
#include "swrast/swrast.h"

/* external functions */
BOOL DiveFlush(PWMC pwc);
int DivePMInit(WMesaContext c);

void
_swrast_CopyPixels( GLcontext *ctx,
                   GLint srcx, GLint srcy, GLsizei width, GLsizei height,
                   GLint destx, GLint desty,
                   GLenum type );

/**********************/
/* internal functions */
/**********************/
static void OS2mesa_update_state( GLcontext* ctx, GLuint new_state  );
BOOL wmFlush(PWMC pwc);
void wmSetPixel(PWMC pwc, int iScanLine, int iPixel, BYTE r, BYTE g, BYTE b);
void wmSetPixel_db1(PWMC pwc, int iScanLine, int iPixel, BYTE r, BYTE g, BYTE b);
void wmSetPixel_db2(PWMC pwc, int iScanLine, int iPixel, BYTE r, BYTE g, BYTE b);
void wmSetPixel_db3(PWMC pwc, int iScanLine, int iPixel, BYTE r, BYTE g, BYTE b);
void wmSetPixel_db4(PWMC pwc, int iScanLine, int iPixel, BYTE r, BYTE g, BYTE b);
void wmSetPixel_sb(PWMC pwc, int iScanLine, int iPixel, BYTE r, BYTE g, BYTE b);
int wmGetPixel(PWMC pwc, int x, int y);

void wmCreateDIBSection(
                        HDC  hDC,
                        PWMC pwc,   // handle of device context
                        CONST BITMAPINFO2 *pbmi// address of structure containing bitmap size, format, and color data
                       );
                        //, UINT iUsage) // color data type indicator: RGB values or palette indices

BYTE DITHER_RGB_2_8BIT( int r, int g, int b, int x, int y);

/*******************/
/* macros          */
/*******************/
#define LONGFromRGB(R,G,B) (LONG)(((LONG)R<<16)+((LONG)G<<8)+(LONG)B)
/* Windos use inverse order for Red and Blue */
#define GetRValue(rgb)     ((BYTE)((rgb)>>16))
#define GetGValue(rgb)     ((BYTE)((rgb)>> 8))
#define GetBValue(rgb)     ((BYTE)(rgb))
#define MAKEWORD(a, b)       ((short int)(((BYTE)(a)) | (((short int)((BYTE)(b))) << 8)))
/*
* Useful macros:
Modified from file osmesa.c
*/

//#define PIXELADDR(X,Y)  ((GLubyte *)Current->pbPixels + (Current->height-Y-1)* Current->ScanWidth + (X)*nBypp)
#define PIXELADDR(X,Y)  ((GLubyte *)Current->pbPixels + (Y)* Current->ScanWidth + (X)*nBypp)
#define PIXELADDR1( X, Y )  \
((GLubyte *)wmesa->pbPixels + (Y) * wmesa->ScanWidth + (X))
//((GLubyte *)wmesa->pbPixels + (wmesa->height-Y-1) * wmesa->ScanWidth + (X))
#define PIXELADDR2( X, Y )  \
((GLubyte *)wmesa->pbPixels + (Y) * wmesa->ScanWidth + (X)*2)
//((GLubyte *)wmesa->pbPixels + (wmesa->height - Y - 1) * wmesa->ScanWidth + (X)*2)
#define PIXELADDR4( X, Y )  \
((GLubyte *)wmesa->pbPixels + (Y) * wmesa->ScanWidth + (X)*4)
//((GLubyte *)wmesa->pbPixels + (wmesa->height-Y-1) * wmesa->ScanWidth + (X)*4)


#define PIXELADDR_1( X, Y )  \
((GLubyte *)pwc->pbPixels + (Y) * pwc->ScanWidth + (X))
#define PIXELADDR_2( X, Y )  \
((GLubyte *)pwc->pbPixels + (Y) * pwc->ScanWidth + (X)*2)
#define PIXELADDR_3( X, Y )  \
((GLubyte *)pwc->pbPixels + (Y) * pwc->ScanWidth + (X)*3)
#define PIXELADDR_4( X, Y )  \
((GLubyte *)pwc->pbPixels + (Y) * pwc->ScanWidth + (X)*4)


/*
 * Values for the format parameter of OSMesaCreateContext()
 */
#define OSMESA_COLOR_INDEX GL_COLOR_INDEX
#define OSMESA_RGBA        GL_RGBA
#define OSMESA_BGRA        0x1
#define OSMESA_ARGB        0x2
#define OSMESA_RGB         GL_RGB
#define OSMESA_BGR         0x4
#define OSMESA_RGB_565     0x5


/*******************/
/* local constants */
/*******************/

/* static */
PWMC Current = NULL;
GLenum stereoCompile = GL_FALSE ;
GLenum stereoShowing  = GL_FALSE ;
GLenum stereoBuffer = GL_FALSE;

GLint stereo_flag = 0 ;

static void wmSetPixelFormat( PWMC wc, HDC hDC)
{
    if(wc->rgb_flag)
    {  long Alarray[4];
       long lFormats[24];/* Formats supported by the device    */
       int Nformats;
/*
  { int i;
    long cAlarray[104];
    FILE *fp;

    i = 64;
    DevQueryCaps(hDC,CAPS_FAMILY,i, cAlarray);
    fp=fopen("test.txt","w");
    for(i=0;i<64;i++)
      fprintf(fp,"%2i, %x\n",i,cAlarray[i]);
    fclose(fp);
  }
*/

        DevQueryCaps(hDC,CAPS_BITMAP_FORMATS,1, Alarray);
         Nformats = (int)Alarray[0];
       if(Nformats > 12) Nformats=12;

 /* Get screen supportable formats */
 GpiQueryDeviceBitmapFormats(wc->hps, Nformats*2, lFormats);
// pbmInfo.cPlanes = (USHORT) lFormats[0] ;
// pbmInfo.cBitCount = (USHORT) lFormats[1];

         DevQueryCaps(hDC,CAPS_COLOR_BITCOUNT,1, Alarray);
        wc->nColorBits = (int) Alarray[0];
      printf("CAPS_COLOR_BITCOUNT=%i ",wc->nColorBits);

      if(wc->nColorBits == 24 && wc->useDive && (wc->DiveCaps.ulDepth == 32)) /* Matrox driver */
                   wc->nColorBits = 32;
      else
      {   if(wc->nColorBits == 8 /* && wc->useDive */ )
          {   wc->nColorBits = 24;
             // wc->useDive = 0;
          } else if(wc->nColorBits < 24){
                wc->nColorBits = 24;
          } else if(wc->nColorBits == 32){
/* Вещь, специфическая для PM'а ?*/
//             wc->nColorBits = 24;
                wc->nColorBits = 32;
          }
      }

//             else if(wc->nColorBits == 16 && wc->useDive)
//          {   wc->nColorBits = 24;
//          }
    } else
        wc->nColorBits = 8;

    printf("wc->nColorBits=%i\n",wc->nColorBits);

    switch(wc->nColorBits){
    case 8:
        if(wc->dither_flag != GL_TRUE)
            wc->pixelformat = PF_INDEX8;
        else
            wc->pixelformat = PF_DITHER8;
        break;
    case 16:
        wc->pixelformat = PF_5R6G5B;
        break;
    case 24:
        wc->pixelformat = PF_8R8G8B;
        break;
    case 32:
        wc->pixelformat = PF_8A8B8G8R;//PF_8R8G8B;
        break;
    default:
        wc->pixelformat = PF_BADFORMAT;
    }
}

//
// This function creates the DIB section that is used for combined
// GL and GDI calls
//
BOOL  wmCreateBackingStore(PWMC pwc, long lxSize, long lySize)
{
    HDC hdc = pwc->hDC;

    BITMAPINFO2   *pbmi_mem = &(pwc->memb.bmi_mem);
    BITMAPINFOHEADER2 *pbmi = &(pwc->bmi);


    pbmi->cbFix = sizeof(BITMAPINFOHEADER2);
    pbmi->cx = lxSize;
    pbmi->cy = lySize;
    pbmi->cPlanes = 1;
    if(pwc->rgb_flag)
    {  LONG *pVC_Caps;
       pVC_Caps = GetVideoConfig(pwc->hDC);
       pbmi->cBitCount = pVC_Caps[CAPS_COLOR_BITCOUNT];
printf("1 pbmi->cBitCount=%i\n",pbmi->cBitCount);
       if(pbmi->cBitCount == 24  && pwc->useDive && (pwc->DiveCaps.ulDepth == 32))   /* Matrox driver */
           pbmi->cBitCount = 32;
       else
       {
           if(pbmi->cBitCount == 8)
           {  if(pwc->useDive&0x0f)
                    pwc->useDive |= 0x200; /* use dithering for DIVE */
              pbmi->cBitCount = 24;
           }  else {
               if(pbmi->cBitCount == 16 && (pwc->useDive&0x0f) )
                                                   pwc->useDive |= 0x100;
//tmpDEBUG      pbmi->cBitCount = 24;
               if(pbmi->cBitCount < 32)
                       pbmi->cBitCount = 24;
           }
       }
    }
      else
        pbmi->cBitCount = 8;

    printf("pbmi->cBitCount=%i, useDive=%x\n",pbmi->cBitCount,pwc->useDive);

    pbmi->ulCompression = BCA_UNCOMP;
    pbmi->cbImage = 0;
    pbmi->cxResolution = 0;
    pbmi->cyResolution = 0;
    pbmi->cclrUsed = 0;
    pbmi->cclrImportant = 0;
      pbmi->usUnits = BRU_METRIC;
      pbmi->usReserved = 0;
      pbmi->usRecording = BRA_BOTTOMUP;
      pbmi->usRendering = BRH_NOTHALFTONED;

      pbmi->cSize1 = 0;
      pbmi->cSize2 = 0;
      pbmi->ulColorEncoding = BCE_RGB;
      pbmi->ulIdentifier = 1;

//    iUsage = (pbmi->cBitCount <= 8) ? DIB_PAL_COLORS : DIB_RGB_COLORS;

    pwc->nColorBits = pbmi->cBitCount;
    pwc->ScanWidth = pwc->pitch = (int)lxSize;
    memcpy((void *)pbmi_mem, (void *)pbmi, sizeof(BITMAPINFOHEADER2));
//    pbmi_mem->argbColor[0] = 0;

    wmCreateDIBSection(hdc, pwc, pbmi_mem);

//    if ((iUsage == DIB_PAL_COLORS) && !(pwc->hGLPalette)) {
//        wmCreatePalette( pwc );
//        wmSetDibColors( pwc );
//    }
    wmSetPixelFormat(pwc, pwc->hDC);

    return(TRUE);
}

//
// Free up the dib section that was created
//
BOOL wmDeleteBackingStore(PWMC pwc)
{
  if(pwc->memb.pBmpBuffer)
  {   free(pwc->memb.pBmpBuffer);
      pwc->memb.pBmpBuffer = NULL;
  }
  pwc->pbPixels = NULL;
  if(pwc->hbm) GpiDeleteBitmap(pwc->hbm);
  pwc->hbm = 0;

  //free( pbmi);

  GpiDestroyPS(pwc->memb.hpsMem);       /* destroys presentation space */
  DevCloseDC(pwc->memb.hdcMem);         /* closes device context       */
  return TRUE;
}



/***********************************************/
/* Return characteristics of the output buffer. */
/***********************************************/
static void get_buffer_size( GLframebuffer *buffer, GLuint *width, GLuint *height )
{
   int New_Size;
   RECTL CR;
   GET_CURRENT_CONTEXT(ctx);
   WinQueryWindowRect(Current->Window,&CR);


   *width = (GLuint) (CR.xRight - CR.xLeft);
   *height = (GLuint) (CR.yTop - CR.yBottom);

   New_Size=((*width)!=Current->width) || ((*height)!=Current->height);

   if (New_Size){
      Current->width=*width;
      Current->height=*height;
      Current->ScanWidth=Current->width;
      if ((Current->ScanWidth%sizeof(long))!=0)
         Current->ScanWidth+=(sizeof(long)-(Current->ScanWidth%sizeof(long)));

      if (Current->db_flag){
         if (Current->rgb_flag==GL_TRUE && Current->dither_flag!=GL_TRUE){
            wmDeleteBackingStore(Current);
            wmCreateBackingStore(Current, Current->width, Current->height);
         }
      }

      //  Resize OsmesaBuffer if in Parallel mode
   }
}

//
// We cache all gl draw routines until a flush is made
//
static void flush(GLcontext* ctx)
{
        if((Current->rgb_flag /*&& !(Current->dib.fFlushed)*/&&!(Current->db_flag))
            ||(!Current->rgb_flag))
        {
            wmFlush(Current);
        }
}

/*
 * Set the color index used to clear the color buffer.
 */
static void clear_index(GLcontext* ctx, GLuint index)
{
        Current->clearpixel = index;
}

/*
 * Set the color used to clear the color buffer.
 */
static void clear_color( GLcontext* ctx, const GLfloat color[4] )
{
  GLubyte col[4];
  CLAMPED_FLOAT_TO_UBYTE(col[0], color[0]);
  CLAMPED_FLOAT_TO_UBYTE(col[1], color[1]);
  CLAMPED_FLOAT_TO_UBYTE(col[2], color[2]);
  Current->clearpixel = LONGFromRGB(col[0], col[1], col[2]);
}


/*
 * Clear the specified region of the color buffer using the clear color
 * or index as specified by one of the two functions above.
 *
 * This procedure clears either the front and/or the back COLOR buffers.
 * Only the "left" buffer is cleared since we are not stereo.
 * Clearing of the other non-color buffers is left to the swrast.
 * We also only clear the color buffers if the color masks are all 1's.
 * Otherwise, we let swrast do it.
 */

static void clear(GLcontext* ctx, GLbitfield mask,
                  GLboolean all, GLint x, GLint y, GLint width, GLint height)
{  RECTL rect;
   if (all)
   {  x=y=0;
      width=Current->width;
      height=Current->height;
   }
  /* clear alpha */
  if ((mask & (DD_FRONT_LEFT_BIT | DD_BACK_RIGHT_BIT)) &&
      ctx->DrawBuffer->UseSoftwareAlphaBuffers &&
      ctx->Color.ColorMask[ACOMP]) {
      _mesa_clear_alpha_buffers( ctx );
  }
   if(Current->db_flag==GL_TRUE)
   {
    int         dwColor;
    short int   wColor;
    BYTE        bColor;
    int         *lpdw = (int *)Current->pbPixels;
    short int   *lpw = (short int *)Current->pbPixels;
    PBYTE       lpb = Current->pbPixels;
    int     lines;


        if(Current->db_flag==GL_TRUE){
            UINT    nBypp = Current->nColorBits / 8;
            int     i = 0;
            int     iSize = 0;

            if(nBypp ==1 ){
                /* Need rectification */
                iSize = Current->width/4;
                bColor  = (BYTE) BGR8(GetRValue(Current->clearpixel),
                    GetGValue(Current->clearpixel),
                    GetBValue(Current->clearpixel));
                wColor  = MAKEWORD(bColor,bColor);
                dwColor = (int) MAKELONG(wColor, wColor);
            }
            if(nBypp == 2){
                 int r,g,b;
                 r = GetRValue(Current->clearpixel);
                 g = GetGValue(Current->clearpixel);
                 b = GetBValue(Current->clearpixel);
                iSize = Current->width / 2;
                wColor = BGR16(GetRValue(Current->clearpixel),
                    GetGValue(Current->clearpixel),
                    GetBValue(Current->clearpixel));
                dwColor = (int)MAKELONG(wColor, wColor);
            }
            else if(nBypp == 4){
                iSize = Current->width;
                dwColor = (int)BGR32(GetRValue(Current->clearpixel),
                    GetGValue(Current->clearpixel),
                    GetBValue(Current->clearpixel));
            }

            while(i < iSize){
                *lpdw = dwColor;
                lpdw++;
                i++;
            }

            //
            // This is the 24bit case
            //
            if (nBypp == 3) {
                iSize = Current->width *3/4;
                dwColor = (int)BGR24(GetRValue(Current->clearpixel),
                    GetGValue(Current->clearpixel),
                    GetBValue(Current->clearpixel));
                while(i < iSize){
                    *lpdw = dwColor;
                    lpb += nBypp;
                    lpdw = (int *)lpb;
                    i++;
                }
            }

            i = 0;
            if (stereo_flag)
               lines = height /2;
            else
               lines = height;
            do {
                memcpy(lpb, Current->pbPixels, iSize*4);
                lpb += Current->ScanWidth;
                i++;
            }
            while (i<lines-1);
        }
        mask &= ~DD_FRONT_LEFT_BIT;
   } else { // For single buffer
      rect.xLeft = x;
      rect.xRight = x + width;
      rect.yBottom = y;
      rect.yTop = y + height;
      WinFillRect(Current->hps, &rect, Current->clearpixel);
      mask &= ~DD_FRONT_LEFT_BIT;
   }

  /* Call swrast if there is anything left to clear (like DEPTH) */
  if (mask)
      _swrast_Clear( ctx, mask, all, x, y, width, height );

}

static void enable( GLcontext* ctx, GLenum pname, GLboolean enable )
{
  if (!Current)
    return;

  if (pname == GL_DITHER) {
    if(enable == GL_FALSE){
      Current->dither_flag = GL_FALSE;
      if(Current->nColorBits == 8)
       Current->pixelformat = PF_INDEX8;
    }
    else{
      if (Current->rgb_flag && Current->nColorBits == 8){
          Current->pixelformat = PF_DITHER8;
          Current->dither_flag = GL_TRUE;
      }
      else
          Current->dither_flag = GL_FALSE;
    }
  }
}

static void set_buffer(GLcontext *ctx, GLframebuffer *colorBuffer,
                       GLuint bufferBit )
{
  /* XXX todo - examine bufferBit and set read/write pointers */
  return;
}


/* Set the current color index. */
static void set_index(GLcontext* ctx, GLuint index)
{
        Current->pixel=index;
}


/* Set the current RGBA color. */
static void set_color( GLcontext* ctx, GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
        Current->pixel = LONGFromRGB( r, g, b );
}


/* Set the index mode bitplane mask. */
static GLboolean index_mask(GLcontext* ctx, GLuint mask)
{
    /* can't implement */
    return GL_FALSE;
}


/* Set the RGBA drawing mask. */
static GLboolean color_mask( GLcontext* ctx,
                            GLboolean rmask, GLboolean gmask,
                            GLboolean bmask, GLboolean amask)
{
    /* can't implement */
    return GL_FALSE;
}


/*
* Set the pixel logic operation.  Return GL_TRUE if the device driver
* can perform the operation, otherwise return GL_FALSE.  If GL_FALSE
* is returned, the logic op will be done in software by Mesa.
*/
GLboolean logicop( GLcontext* ctx, GLenum op )
{
    /* can't implement */
    return GL_FALSE;
}

/**********************************************************************/
/*****                 Span-based pixel drawing                   *****/
/**********************************************************************/

/* Write a horizontal span of 32-bit color-index pixels with a boolean mask. */
static void write_ci32_span( const GLcontext* ctx,
                             GLuint n, GLint x, GLint y,
                             const GLuint index[],
                             const GLubyte mask[] )
{
    GLuint i;
    PBYTE Mem=Current->ScreenMem +  y*Current->ScanWidth + x;
    assert(Current->rgb_flag==GL_FALSE);
    for (i=0; i<n; i++)
        if (mask[i])
            Mem[i]=index[i];
}


/* Write a horizontal span of 8-bit color-index pixels with a boolean mask. */
static void write_ci8_span( const GLcontext* ctx,
                            GLuint n, GLint x, GLint y,
                            const GLubyte index[],
                            const GLubyte mask[] )
{
    GLuint i;
    PBYTE Mem=Current->ScreenMem + y *Current->ScanWidth+x;
    assert(Current->rgb_flag==GL_FALSE);
    for (i=0; i<n; i++)
        if (mask[i])
            Mem[i]=index[i];
}



/*
* Write a horizontal span of pixels with a boolean mask.  The current
* color index is used for all pixels.
*/
static void write_mono_ci_span(const GLcontext* ctx,
                               GLuint n,GLint x,GLint y,
                               GLuint colorIndex, const GLubyte mask[])
{
   GLuint i;
   BYTE *Mem=Current->ScreenMem + y * Current->ScanWidth+x;
   assert(Current->rgb_flag==GL_FALSE);
   for (i=0; i<n; i++)
      if (mask[i])
         Mem[i]= (BYTE) colorIndex;
}

/* Write a horizontal span of RGBA color pixels with a boolean mask. */
static void write_rgba_span( const GLcontext* ctx, GLuint n, GLint x, GLint y,
                             const GLubyte rgba[][4], const GLubyte mask[] )
{
  PWMC    pwc = Current;
   GLuint i;
//  if (pwc->rgb_flag==GL_TRUE)
    {
        PBYTE  lpb = pwc->pbPixels;
        UINT  *lpdw;
        unsigned short int *lpw;
      if(Current->db_flag)
      {
        UINT    nBypp = pwc->nColorBits / 8;
        lpb += pwc->ScanWidth * y;
        // Now move to the desired pixel
        lpb += x * nBypp;
// lpb = (PBYTE)PIXELADDR(x, iScanLine);
//#define PIXELADDR(X,Y)  ((GLubyte *)Current->pbPixels + (Y+1)* Current->ScanWidth + (X)*nBypp)
//                                                          ???

        if (mask) {
            for (i=0; i<n; i++)
                if (mask[i])
                    wmSetPixel(pwc, y, x + i, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
        } else {
// pаскpываем все вызовы
//            for (i=0; i<n; i++)
//                wmSetPixel(pwc, y, x + i, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP] );
           if(nBypp == 1)
           {
              if(pwc->dither_flag)
                 for (i=0; i<n; i++,lpb++)
                              *lpb = DITHER_RGB_2_8BIT(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP],y,x);
              else
                 for (i=0; i<n; i++,lpb++)
                              *lpb = (BYTE) BGR8(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
           } else if(nBypp == 2) {
              lpw = (unsigned short int *)lpb;
                 for (i=0; i<n; i++,lpw++)
                               *lpw = BGR16(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
           } else if (nBypp == 3) {

                 for (i=0; i<n; i++)
                 {  *lpb++ = rgba[i][BCOMP]; // in memory: b-g-r bytes
                    *lpb++ = rgba[i][GCOMP];
                    *lpb++ = rgba[i][RCOMP];
                 }
           } else if (nBypp == 4) {
              lpdw = (UINT  *)lpb;
//???                 memcpy(lpb,rgba,n*4);
                 for (i=0; i<n; i++,lpdw++)
                    *lpdw = ((UINT)rgba[i][BCOMP]) | (((UINT)rgba[i][GCOMP])<<8 )|  (((UINT)rgba[i][RCOMP])<<16);
//                     *lpdw = BGR32(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
//#define BGR32(r,g,b) (unsigned long)((DWORD)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))

           }

        }

      } else {  // (!Current->db_flag)
          POINTL ptl;
          int col=-1, colold=-1,iold=0;
          ptl.y = y;
          ptl.x = x;

          if (mask)
          {
            for (i=0; i<n; i++,ptl.x++)
                if (mask[i])
                {   GpiSetColor(pwc->hps,LONGFromRGB(rgba[i][RCOMP],rgba[i][GCOMP], rgba[i][BCOMP]));
                    GpiSetPel(pwc->hps, &ptl);
                }
          } else {
            GpiMove(pwc->hps,&ptl);
            for (i=0; i<n; i++,ptl.x++)
                {   col = (int) LONGFromRGB(rgba[i][RCOMP],rgba[i][GCOMP], rgba[i][BCOMP]);
                    if(col != colold)
                    {  GpiSetColor(pwc->hps,col);
                       GpiLine(pwc->hps,&ptl);
                       colold = col;
                       iold = i;
                    }
                }
                if(iold != n-1)
                    {  ptl.x--;
                      // GpiSetColor(pwc->hps,col);
                       GpiLine(pwc->hps,&ptl);
                    }
         } //endif(mask)
      } //endif(Current->db_flag)

    }
//  endofif (pwc->rgb_flag==GL_TRUE)
}

/* Write a horizontal span of RGB color pixels with a boolean mask. */
static void write_rgb_span( const GLcontext* ctx,
                            GLuint n, GLint x, GLint y,
                            const GLubyte rgb[][3], const GLubyte mask[] )
{
    PWMC    pwc = Current;
    GLuint i;
    if(pwc->db_flag)
    {
        if (mask) {
            for (i=0; i<n; i++)
                if (mask[i])
                    wmSetPixel(pwc, y, x + i, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP]);
        }
        else {
            for (i=0; i<n; i++)
                wmSetPixel(pwc, y, x + i, rgb[i][RCOMP], rgb[i][GCOMP], rgb[i][BCOMP] );
        }
    } else {  // (!pwc->db_flag)
          POINTL ptl;
          int col=-1, colold=-1,iold=0;
          ptl.y = y;
          ptl.x = x;

          if (mask)
          {
            for (i=0; i<n; i++,ptl.x++)
                if (mask[i])
                {   GpiSetColor(pwc->hps,LONGFromRGB(rgb[i][RCOMP],rgb[i][GCOMP], rgb[i][BCOMP]));
                    GpiSetPel(pwc->hps, &ptl);
                }
          } else {
            GpiMove(pwc->hps,&ptl);
            for (i=0; i<n; i++,ptl.x++)
                {   col = (int) LONGFromRGB(rgb[i][RCOMP],rgb[i][GCOMP], rgb[i][BCOMP]);
                    if(col != colold)
                    {  GpiSetColor(pwc->hps,col);
                       GpiLine(pwc->hps,&ptl);
                       colold = col;
                       iold = i;
                    }
                }
                if(iold != n-1)
                    {  ptl.x--;
                      // GpiSetColor(pwc->hps,col);
                       GpiLine(pwc->hps,&ptl);
                    }
         } //endif(mask)
      } //endif(pwc->db_flag)
}

/*
* Write a horizontal span of pixels with a boolean mask all with the same color
*/
static void write_mono_rgba_span( const GLcontext* ctx,
                                  GLuint n, GLint x, GLint y,
                                  const GLchan color[4], const GLubyte mask[])
{
    GLuint i;
    PWMC pwc = Current;
    assert(Current->rgb_flag==GL_TRUE);
    if(pwc->db_flag)
    { if(mask)
      { for (i=0; i<n; i++)
            if (mask[i])
                 wmSetPixel(pwc,y,x+i,color[RCOMP], color[GCOMP], color[BCOMP]);
      } else {
        for (i=0; i<n; i++)
                 wmSetPixel(pwc,y,x+i,color[RCOMP], color[GCOMP], color[BCOMP]);
      }
    } else {
     POINTL   Point;  /*  Position in world coordinates. */
     LONG l_Color = LONGFromRGB(color[RCOMP], color[GCOMP], color[BCOMP]);

        Point.y = y;
        GpiSetColor(pwc->hps,l_Color);
        if(mask)
        { for (i=0; i<n; i++)
            if (mask[i])
            {  Point.x = x+i;
               GpiSetPel(pwc->hps, &Point);
            }
        } else {
            Point.x = x;
            if(n <= 1)   GpiSetPel(pwc->hps, &Point);
            else
            {  GpiMove(pwc->hps,&Point);
               Point.x += n-1;
               GpiLine(pwc->hps,&Point);
            }
        }
    }
}
/*************************************************/
/*** optimized N-bytes per pixel functions    ****/
/*************************************************/
//static void write_clear_rgba_span_4rgb_db( const GLcontext* ctx, GLuint n, GLint x, GLint y,
//                             const GLubyte rgba[4])

static void write_mono_rgba_span_4rgb_db( const GLcontext* ctx,
                                  GLuint n, GLint x, GLint y,
                                  const GLchan color[4], const GLubyte mask[])
{
    PWMC    pwc = Current;
    GLuint i;
    PBYTE  lpb = pwc->pbPixels;
    UINT  *lpdw, col;
        // Now move to the desired pixel
    lpdw = (UINT  *)(lpb + pwc->ScanWidth * y);
    lpdw += x;
    col = ((UINT)color[BCOMP]) | (((UINT)color[GCOMP])<<8 ) |
          (((UINT)color[RCOMP])<<16)| (((UINT)color[ACOMP])<<24);

     if (mask)
     {  for (i=0; i<n; i++)
              if (mask[i])
              {  lpdw[i] = col;
              }
     } else {
         for (i=0; i<n; i++,lpdw++)  *lpdw = col;
     }
}

//static void write_clear_rgba_span_3rgb_db( const GLcontext* ctx, GLuint n, GLint x, GLint y,
//                             const GLubyte rgba[4])
static void write_mono_rgba_span_3rgb_db( const GLcontext* ctx,
                                  GLuint n, GLint x, GLint y,
                                  const GLchan color[4], const GLubyte mask[])
{

    PWMC    pwc = Current;
    GLuint i;
    PBYTE  lpb = pwc->pbPixels, pixel;
    UINT  *lpdw, col;
        // Now move to the desired pixel
    lpb += pwc->ScanWidth * y +x*3;
    if (mask)
    {  for (i=0; i<n; i++)
         if (mask[i])
          { pixel = lpb+i*3;
            pixel[0] = color[2];
            pixel[1] = color[1];
            pixel[2] = color[0];
          }

    } else {
      col = ((UINT)color[BCOMP]) | (((UINT)color[GCOMP])<<8 ) |  (((UINT)color[RCOMP])<<16);
       for (i=0; i<n-1; i++,lpb+=3)
       { lpdw = (UINT  *)lpb;
         *lpdw = col;
       }
       lpdw = (UINT  *)lpb;
       *lpdw = col | ((*lpdw)&0xff0000);
    }

}

/* Write a horizontal span of RGBA color pixels with a boolean mask. */
static void write_rgba_span_4rgb_db( const GLcontext* ctx, GLuint n, GLint x, GLint y,
                             const GLubyte rgba[][4], const GLubyte mask[] )
{
    PWMC    pwc = Current;
    GLuint i;
    PBYTE  lpb = pwc->pbPixels;
    UINT  *lpdw;
    lpb += pwc->ScanWidth * y;
        // Now move to the desired pixel
    lpb += x * 4;

     if (mask)
     {
            for (i=0; i<n; i++)
                if (mask[i])
                    wmSetPixel_db4(pwc, y, x + i, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
     } else {
        lpdw = (UINT  *)lpb;
//???                 memcpy(lpb,rgba,n*4);
                 for (i=0; i<n; i++,lpdw++)
                    *lpdw = ((UINT)rgba[i][BCOMP]) | (((UINT)rgba[i][GCOMP])<<8 )|  (((UINT)rgba[i][RCOMP])<<16);
//                     *lpdw = BGR32(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
//#define BGR32(r,g,b) (unsigned long)((DWORD)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))
     }

}

/* Write a horizontal span of RGBA color pixels with a boolean mask. */
static void write_rgba_span_3rgb_db( const GLcontext* ctx, GLuint n, GLint x, GLint y,
                             const GLubyte rgba[][4], const GLubyte mask[] )
{
    PWMC    pwc = Current;
    GLuint i;
    PBYTE  lpb = pwc->pbPixels;
    UINT  *lpdw;
    lpb += pwc->ScanWidth * y;
        // Now move to the desired pixel
    lpb += x * 3;

     if (mask)
     {
            for (i=0; i<n; i++)
                if (mask[i])
                    wmSetPixel_db3(pwc, y, x + i, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
     } else {
           for (i=0; i<n; i++)
            {
///                *lpdw = ((UINT)rgba[i][BCOMP]) | (((UINT)rgba[i][GCOMP])<<8 )|  (((UINT)rgba[i][RCOMP])<<16);
                   *lpb++ = rgba[i][BCOMP]; // in memory: b-g-r bytes
                   *lpb++ = rgba[i][GCOMP];
                   *lpb++ = rgba[i][RCOMP];
            }
//                     *lpdw = BGR32(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
//#define BGR32(r,g,b) (unsigned long)((DWORD)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))
     }

}
/* Write a horizontal span of RGBA color pixels with a boolean mask. */
static void write_rgba_span_2rgb_db( const GLcontext* ctx, GLuint n, GLint x, GLint y,
                             const GLubyte rgba[][4], const GLubyte mask[] )
{
    PWMC    pwc = Current;
    GLuint i;
    PBYTE  lpb = pwc->pbPixels;
    UINT  *lpdw;
    unsigned short int *lpw;

    lpb += pwc->ScanWidth * y;
        // Now move to the desired pixel
    lpb += x * 2;

     if (mask)
     {
            for (i=0; i<n; i++)
                if (mask[i])
                    wmSetPixel_db2(pwc, y, x + i, rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
     } else {
               lpw = (unsigned short int  *)lpb;
                 for (i=0; i<n; i++,lpw++)
                      *lpw = BGR16(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
     }

}


/**********************************************************************/
/*****                   Array-based pixel drawing                *****/
/**********************************************************************/

/* Write an array of 32-bit index pixels with a boolean mask. */
static void write_ci32_pixels( const GLcontext* ctx,
                               GLuint n, const GLint x[], const GLint y[],
                               const GLuint index[], const GLubyte mask[] )
{
   GLuint i;
   assert(Current->rgb_flag==GL_FALSE);
   for (i=0; i<n; i++) {
      if (mask[i]) {
         BYTE *Mem=Current->ScreenMem + y[i] * Current->ScanWidth + x[i];
         *Mem = index[i];
      }
   }
}


/*
* Write an array of pixels with a boolean mask.  The current color
* index is used for all pixels.
*/
static void write_mono_ci_pixels( const GLcontext* ctx,
                                  GLuint n,
                                  const GLint x[], const GLint y[],
                                  GLuint colorIndex, const GLubyte mask[] )
{
   GLuint i;
   assert(Current->rgb_flag==GL_FALSE);
   for (i=0; i<n; i++) {
      if (mask[i]) {
         BYTE *Mem=Current->ScreenMem + y[i] * Current->ScanWidth + x[i];
         *Mem = (BYTE)  colorIndex;
      }
   }
}



/* Write an array of RGBA pixels with a boolean mask. */
static void write_rgba_pixels( const GLcontext* ctx,
                               GLuint n, const GLint x[], const GLint y[],
                               const GLubyte rgba[][4], const GLubyte mask[] )
{
        GLuint i;
    PWMC    pwc = Current;
//  HDC DC=DD_GETDC;
    assert(Current->rgb_flag==GL_TRUE);
    for (i=0; i<n; i++)
       if (mask[i])
          wmSetPixel(pwc, y[i],x[i],rgba[i][RCOMP],rgba[i][GCOMP],rgba[i][BCOMP]);
//  DD_RELEASEDC;
}



/*
* Write an array of pixels with a boolean mask.  The current color
* is used for all pixels.
*/
static void write_mono_rgba_pixels( const GLcontext* ctx,
                                    GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLchan color[4],
                                    const GLubyte mask[] )
{
    GLuint i;
    PWMC    pwc = Current;
//  HDC DC=DD_GETDC;
    assert(Current->rgb_flag==GL_TRUE);
    for (i=0; i<n; i++)
        if (mask[i])
             wmSetPixel(pwc, y[i],x[i],color[RCOMP],
                    color[GCOMP], color[BCOMP]);
//  DD_RELEASEDC;
}

/**********************************************************************/
/*****            Read spans/arrays of pixels                     *****/
/**********************************************************************/

/* Read a horizontal span of color-index pixels. */
static void read_ci32_span( const GLcontext* ctx, GLuint n, GLint x, GLint y,
                            GLuint index[])
{
   GLuint i;
   BYTE *Mem=Current->ScreenMem + y *Current->ScanWidth+x;
   assert(Current->rgb_flag==GL_FALSE);
   for (i=0; i<n; i++)
      index[i]=Mem[i];
}




/* Read an array of color index pixels. */
static void read_ci32_pixels( const GLcontext* ctx,
                              GLuint n, const GLint x[], const GLint y[],
                              GLuint indx[], const GLubyte mask[] )
{
   GLuint i;
   assert(Current->rgb_flag==GL_FALSE);
   for (i=0; i<n; i++) {
      if (mask[i]) {
         indx[i]=*(Current->ScreenMem + y[i] * Current->ScanWidth+x[i]);
      }
   }
}



/* Read a horizontal span of color pixels. */
static void read_rgba_span( const GLcontext* ctx,
                            GLuint n, GLint x, GLint y,
                            GLubyte rgba[][4] )
{
   UINT i;
   LONG Color;
   PWMC pwc = Current;

   if(Current->db_flag)
   {    UINT    nBypp = pwc->nColorBits / 8;
        PBYTE  lpb = pwc->pbPixels;
        UINT  *lpdw;
        unsigned short int *lpw;
        lpb += pwc->ScanWidth * y;
        lpb += x * nBypp;
/*********************/
           if(nBypp == 1)
           {
                 for (i=0; i<n; i++,lpb++)
                 {     Color = (int)   *lpb;
                       rgba[i][RCOMP] = GetRValue(Color);
                       rgba[i][GCOMP] = GetGValue(Color);
                       rgba[i][BCOMP] = GetBValue(Color);
                       rgba[i][ACOMP] = 255;
                 }
           } else if(nBypp == 2) {
              lpw = (unsigned short int *)lpb;
                 for (i=0; i<n; i++,lpw++)
                 {     Color = (int) (*lpw);
                       rgba[i][RCOMP] = GetRValue(Color);
                       rgba[i][GCOMP] = GetGValue(Color);
                       rgba[i][BCOMP] = GetBValue(Color);
                       rgba[i][ACOMP] = 255;
                 }
           } else if (nBypp == 3) {

                 for (i=0; i<n; i++,lpb+=3)
                 {  // GLubyte rtst[4];
                     lpdw = (UINT  *)lpb;
//                        Color = (int) (*lpdw);
//                      *((int *)&rtst) = Color;
//                      rtst[RCOMP] = GetRValue(Color);
//                      rtst[GCOMP] = GetGValue(Color);
//                      rtst[BCOMP] = GetBValue(Color);
//                      rtst[ACOMP] = 255;
                      *((int *)&rgba[i]) = (int) (*lpdw);
//                       rgba[i][RCOMP] = GetRValue(Color);
//                       rgba[i][GCOMP] = GetGValue(Color);
//                       rgba[i][BCOMP] = GetBValue(Color);
                       rgba[i][ACOMP] = 255;
                 }

  //                  *lpdw = BGR24(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
           } else if (nBypp == 4) {
                lpdw = (UINT  *)lpb;
                 for (i=0; i<n; i++,lpdw++)
                 {     Color = (int) (*lpdw);
                       rgba[i][RCOMP] = GetRValue(Color);
                       rgba[i][GCOMP] = GetGValue(Color);
                       rgba[i][BCOMP] = GetBValue(Color);
                       rgba[i][ACOMP] = 255;
                 }

//                    *lpdw = ((UINT)rgba[i][BCOMP]) | (((UINT)rgba[i][GCOMP])<<8 )|  (((UINT)rgba[i][RCOMP])<<16);
//                     *lpdw = BGR32(rgba[i][RCOMP], rgba[i][GCOMP], rgba[i][BCOMP]);
//#define BGR32(r,g,b) (unsigned long)((DWORD)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))

           }

/*********************/
   } else {
     for (i=0; i<n; i++,x++)
     {
       Color = wmGetPixel(pwc,x, y); // GpiQueryPel(Current->hps, &Point);
       rgba[i][RCOMP] = GetRValue(Color);
       rgba[i][GCOMP] = GetGValue(Color);
       rgba[i][BCOMP] = GetBValue(Color);
       rgba[i][ACOMP] = 255;
     }
   }
}


/* Read an array of color pixels. */
static void read_rgba_pixels( const GLcontext* ctx,
                              GLuint n, const GLint x[], const GLint y[],
                              GLubyte rgba[][4], const GLubyte mask[] )
{
   GLuint i;
   LONG Color;
   PWMC pwc = Current;

   assert(Current->rgb_flag==GL_TRUE);

   for (i=0; i<n; i++) {
      if (mask[i]) {
//        Color = GpiQueryPel(Current->hps, &Point);
        Color = wmGetPixel(pwc,x[i], y[i]); // GpiQueryPel(Current->hps, &Point);
        rgba[i][RCOMP] = GetRValue(Color);
         rgba[i][GCOMP] = GetGValue(Color);
         rgba[i][BCOMP] = GetBValue(Color);
         rgba[i][ACOMP] = 255;
//??         rgba[i][ACOMP] = 0;
      }
   }
}



static const GLubyte *OS2get_string( GLcontext *ctx, GLenum name )
{
   switch (name)
   {  case GL_RENDERER:
         return (const GLubyte *) "WarpMesaGL OS/2 PM GPI";
      case GL_VENDOR:
         return (const GLubyte *) "Evgeny Kotsuba";
      default:
         return NULL;
   }
}

static void SetFunctionPointers(GLcontext *ctx)
{
  struct swrast_device_driver *swdd = _swrast_GetDeviceDriverReference( ctx );
  ctx->Driver.GetString = OS2get_string;
// ctx->Driver.UpdateState = wmesa_update_state;
   ctx->Driver.UpdateState = OS2mesa_update_state;
  ctx->Driver.ResizeBuffers = _swrast_alloc_buffers;
  ctx->Driver.GetBufferSize = get_buffer_size;

  ctx->Driver.Accum = _swrast_Accum;
  ctx->Driver.Bitmap = _swrast_Bitmap;
  ctx->Driver.Clear = clear;

// warning EDC0068: Operation between types
// "void(* _Optlink)(struct __GLcontextRec*,unsigned int,unsigned char,int,int,int,int)" and
// "unsigned int(* _Optlink)(struct __GLcontextRec*,unsigned int,unsigned char,int,int,int,int)" is not allowed.

  ctx->Driver.Flush = flush;
  ctx->Driver.ClearIndex = clear_index;
  ctx->Driver.ClearColor = clear_color;

// warning EDC0068: Operation between types
//"void(* _Optlink)(struct __GLcontextRec*,const float*)" and
//"void(* _Optlink)(struct __GLcontextRec*,unsigned char,unsigned char,unsigned char,unsigned char)" is not allowed.
  ctx->Driver.Enable = enable;

  ctx->Driver.CopyPixels = _swrast_CopyPixels;
  ctx->Driver.DrawPixels = _swrast_DrawPixels;
  ctx->Driver.ReadPixels = _swrast_ReadPixels;
  ctx->Driver.DrawBuffer = _swrast_DrawBuffer;

  ctx->Driver.ChooseTextureFormat = _mesa_choose_tex_format;
  ctx->Driver.TexImage1D = _mesa_store_teximage1d;
  ctx->Driver.TexImage2D = _mesa_store_teximage2d;
  ctx->Driver.TexImage3D = _mesa_store_teximage3d;
  ctx->Driver.TexSubImage1D = _mesa_store_texsubimage1d;
  ctx->Driver.TexSubImage2D = _mesa_store_texsubimage2d;
  ctx->Driver.TexSubImage3D = _mesa_store_texsubimage3d;
  ctx->Driver.TestProxyTexImage = _mesa_test_proxy_teximage;

  ctx->Driver.CompressedTexImage1D = _mesa_store_compressed_teximage1d;
  ctx->Driver.CompressedTexImage2D = _mesa_store_compressed_teximage2d;
  ctx->Driver.CompressedTexImage3D = _mesa_store_compressed_teximage3d;
  ctx->Driver.CompressedTexSubImage1D = _mesa_store_compressed_texsubimage1d;
  ctx->Driver.CompressedTexSubImage2D = _mesa_store_compressed_texsubimage2d;
  ctx->Driver.CompressedTexSubImage3D = _mesa_store_compressed_texsubimage3d;

  ctx->Driver.CopyTexImage1D = _swrast_copy_teximage1d;
  ctx->Driver.CopyTexImage2D = _swrast_copy_teximage2d;
  ctx->Driver.CopyTexSubImage1D = _swrast_copy_texsubimage1d;
  ctx->Driver.CopyTexSubImage2D = _swrast_copy_texsubimage2d;
  ctx->Driver.CopyTexSubImage3D = _swrast_copy_texsubimage3d;
  ctx->Driver.CopyColorTable = _swrast_CopyColorTable;
  ctx->Driver.CopyColorSubTable = _swrast_CopyColorSubTable;
  ctx->Driver.CopyConvolutionFilter1D = _swrast_CopyConvolutionFilter1D;
  ctx->Driver.CopyConvolutionFilter2D = _swrast_CopyConvolutionFilter2D;

  swdd->SetBuffer = set_buffer;

  /* Pixel/span writing functions: */
  swdd->WriteRGBASpan        = write_rgba_span;
  swdd->WriteRGBSpan         = write_rgb_span;
  swdd->WriteMonoRGBASpan    = write_mono_rgba_span;
  swdd->WriteRGBAPixels      = write_rgba_pixels;
  swdd->WriteMonoRGBAPixels  = write_mono_rgba_pixels;
  swdd->WriteCI32Span        = write_ci32_span;
  swdd->WriteCI8Span         = write_ci8_span;
  swdd->WriteMonoCISpan      = write_mono_ci_span;
  swdd->WriteCI32Pixels      = write_ci32_pixels;
  swdd->WriteMonoCIPixels    = write_mono_ci_pixels;

  swdd->ReadCI32Span        = read_ci32_span;
  swdd->ReadRGBASpan        = read_rgba_span;
  swdd->ReadCI32Pixels      = read_ci32_pixels;
  swdd->ReadRGBAPixels      = read_rgba_pixels;

//  switch(ctx->DriverCtx.format)
//  {  case OSMESA_RGB:
//       break;
//  }


}
static void OS2mesa_update_state4( GLcontext* ctx, GLuint new_state   )
{
   struct swrast_device_driver *swdd = _swrast_GetDeviceDriverReference( ctx );
   OS2mesa_update_state( ctx,new_state );
   ctx->Driver.UpdateState = OS2mesa_update_state4;
   swdd->WriteRGBASpan        = write_rgba_span_4rgb_db;
   swdd->WriteMonoRGBASpan    = write_mono_rgba_span_4rgb_db;
//   swdd->WriteClearRGBASpan   = write_clear_rgba_span_4rgb_db;
}

static void OS2mesa_update_state3( GLcontext* ctx, GLuint new_state   )
{
   struct swrast_device_driver *swdd = _swrast_GetDeviceDriverReference( ctx );
   OS2mesa_update_state( ctx,new_state );
   ctx->Driver.UpdateState = OS2mesa_update_state3;
   swdd->WriteRGBASpan        = write_rgba_span_3rgb_db;
   swdd->WriteMonoRGBASpan    = write_mono_rgba_span_3rgb_db;
//   swdd->WriteClearRGBASpan   = write_clear_rgba_span_3rgb_db;
}

static void OS2mesa_update_state2( GLcontext* ctx, GLuint new_state   )
{
   struct swrast_device_driver *swdd = _swrast_GetDeviceDriverReference( ctx );
   OS2mesa_update_state( ctx,new_state );
   ctx->Driver.UpdateState = OS2mesa_update_state2;
   swdd->WriteRGBASpan        = write_rgba_span_2rgb_db;
//   swdd->WriteClearRGBASpan   = write_clear_rgba_span_2rgb_db;
}

static void OS2mesa_update_state1( GLcontext* ctx, GLuint new_state   )
{
//   struct swrast_device_driver *swdd = _swrast_GetDeviceDriverReference( ctx );
   OS2mesa_update_state( ctx,new_state );
   ctx->Driver.UpdateState = OS2mesa_update_state1;
//   ctx->Driver.WriteRGBASpan        = write_rgba_span_3rgb_db;
//   ctx->Driver.WriteClearRGBASpan   = write_clear_rgba_span_3rgb_db;
}

static void OS2mesa_update_state( GLcontext* ctx, GLuint new_state  )
{
  struct swrast_device_driver *swdd = _swrast_GetDeviceDriverReference( ctx );
  TNLcontext *tnl = TNL_CONTEXT(ctx);

  /*
   * XXX these function pointers could be initialized just once during
   * context creation since they don't depend on any state changes.
   * kws - This is true - this function gets called a lot and it
   * would be good to minimize setting all this when not needed.
   */
#ifndef SET_FPOINTERS_ONCE
  SetFunctionPointers(ctx);
#endif //  !SET_FPOINTERS_ONCE
  tnl->Driver.RunPipeline = _tnl_run_pipeline;

  _swrast_InvalidateState( ctx, new_state );
  _swsetup_InvalidateState( ctx, new_state );
  _ac_InvalidateState( ctx, new_state );
  _tnl_InvalidateState( ctx, new_state );

}
/***********************************************/

WMesaContext WMesaCreateContext
        ( HWND hWnd, HPAL* Pal, HPS   hpsCurrent, HAB hab,
          GLboolean rgb_flag, GLboolean db_flag, int useDive )
//todo ??                               GLboolean alpha_flag )
{
    RECTL CR;
    WMesaContext c;
    GLboolean true_color_flag;
    UINT  nBypp;
    int rc;

    c = (struct wmesa_context * ) calloc(1,sizeof(struct wmesa_context));
    if (!c)
        return NULL;

    c->Window=hWnd;
    c->hDC = WinQueryWindowDC(hWnd);
    c->hps = hpsCurrent;
    c->hab = hab;
    if(!db_flag)
        useDive = 0; /* DIVE can be used only with db */

    c->hDive = NULLHANDLE;
//printf("2 useDive=%i\n",useDive);
    if(useDive)
    {       rc = DivePMInit(c);
            if(rc)
            { useDive = 0;
            }
    }
    c->useDive = useDive;
//??    true_color_flag = GetDeviceCaps(c->hDC, BITSPIXEL) > 8;
    true_color_flag = 1;
    c->dither_flag = GL_FALSE;
#ifdef DITHER
    if ((true_color_flag==GL_FALSE) && (rgb_flag == GL_TRUE)){
        c->dither_flag = GL_TRUE;
        c->hPalHalfTone = WinGCreateHalftonePalette();
    }
    else
        c->dither_flag = GL_FALSE;
#else
    c->dither_flag = GL_FALSE;
#endif


    if (rgb_flag==GL_FALSE)
    {
        c->rgb_flag = GL_FALSE;
        //    c->pixel = 1;
        c->db_flag = db_flag =GL_TRUE; // WinG requires double buffering
        printf("Single buffer is not supported in color index mode, setting to double buffer.\n");
    }
    else
    {
        c->rgb_flag = GL_TRUE;
        //    c->pixel = 0;
    }
    WinQueryWindowRect(c->Window,&CR);
    c->width=CR.xRight - CR.xLeft;
    c->height=CR.yTop - CR.yBottom;
    if (db_flag)
    {
        c->db_flag = 1;
        /* Double buffered */
#ifndef DDRAW
        //  if (c->rgb_flag==GL_TRUE && c->dither_flag != GL_TRUE )
        {
            wmCreateBackingStore(c, c->width, c->height);

        }
#endif
    }
    else
    {
        /* Single Buffered */
        if (c->rgb_flag)
            c->db_flag = 0;
    }

#ifdef DDRAW
    if (DDInit(c,hWnd) == GL_FALSE) {
        free( (void *) c );
        exit(1);
    }
#endif


    c->gl_visual = _mesa_create_visual(rgb_flag,
                                    db_flag,    /* db_flag */
                                    GL_FALSE,   /* stereo */
                                    8,8,8,8,    /* r, g, b, a bits , todo: alpha_flag ? 8 : 0,  alpha bits */
                                    0,          /* index bits */
                                    16,         /* depth_bits */
                                    8,          /* stencil_bits */
                                    16,16,16,/* accum_bits */
                                    16,      /* todo:  alpha_flag ? 16 : 0,  alpha accum */
                                    1);

    if (!c->gl_visual) {
        return NULL;
    }
    /* allocate a new Mesa context */
    c->gl_ctx = _mesa_create_context( c->gl_visual, NULL, (void *) c, GL_FALSE );


    if (!c->gl_ctx) {
       _mesa_destroy_visual( c->gl_visual );
        free(c);
        return NULL;
    }

  _mesa_enable_sw_extensions(c->gl_ctx);
  _mesa_enable_1_3_extensions(c->gl_ctx);
  _mesa_enable_1_4_extensions(c->gl_ctx);

  c->gl_buffer = _mesa_create_framebuffer( c->gl_visual,
                          c->gl_visual->depthBits > 0,
                          c->gl_visual->stencilBits > 0,
                          c->gl_visual->accumRedBits > 0,
                          1  /* alpha_flag  s/w alpha */ );
  if (!c->gl_buffer) {
    _mesa_destroy_visual( c->gl_visual );
    _mesa_free_context_data( c->gl_ctx );
    free(c);
    return NULL;
  }


    nBypp = c->nColorBits / 8;
    if(useDive)
    {
    printf("\aTodo Dive\n");
    exit(1);
//        if(nBypp == 4)
//          c->gl_ctx->Driver.UpdateState = OS2Dive_mesa_update_state4;
//        else
//            if(nBypp == 3)
//          c->gl_ctx->Driver.UpdateState = OS2Dive_mesa_update_state3;
//        else
//            if(nBypp == 2)
//          c->gl_ctx->Driver.UpdateState = OS2Dive_mesa_update_state2;
//        else
//          c->gl_ctx->Driver.UpdateState = OS2Dive_mesa_update_state1;
    } else {
       if(db_flag && rgb_flag)
       {
          if(nBypp == 4)
             c->gl_ctx->Driver.UpdateState = OS2mesa_update_state4;
          else
            if(nBypp == 3)
                 c->gl_ctx->Driver.UpdateState = OS2mesa_update_state3;
          else
            if(nBypp == 2)
                 c->gl_ctx->Driver.UpdateState = OS2mesa_update_state2;
          else
                 c->gl_ctx->Driver.UpdateState = OS2mesa_update_state1;
       }
         else
             c->gl_ctx->Driver.UpdateState = OS2mesa_update_state;
    }

  /* Initialize the software rasterizer and helper modules.
   */
  {
    GLcontext *ctx = c->gl_ctx;
    _swrast_CreateContext( ctx );
    _ac_CreateContext( ctx );
    _tnl_CreateContext( ctx );
    _swsetup_CreateContext( ctx );

#ifdef SET_FPOINTERS_ONCE
    SetFunctionPointers(ctx);
#endif // SET_FPOINTERS_ONCE
    _swsetup_Wakeup( ctx );
  }
#ifdef COMPILE_SETPIXEL
  ChooseSetPixel(c);
#endif


  return c;
}


void WMesaDestroyContext( void )
{
}



void WMesaMakeCurrent( WMesaContext c )
{
  if(!c){
    Current = c;
    return;
  }

  if(Current == c)
    return;

  Current = c;
  c->gl_ctx->Driver.UpdateState(c->gl_ctx,0);
  _mesa_make_current(c->gl_ctx, c->gl_buffer);


  if (Current->gl_ctx->Viewport.Width==0) {
    /* initialize viewport to window size */
    _mesa_Viewport( 0, 0, Current->width, Current->height );
    Current->gl_ctx->Scissor.Width = Current->width;
    Current->gl_ctx->Scissor.Height = Current->height;
  }
  if ((c->nColorBits <= 8 ) && (c->rgb_flag == GL_TRUE)){
    WMesaPaletteChange(c->hPalHalfTone);
  }

}

void WMesaSwapBuffers( void )
{
//    HDC DC = Current->hDC;
    if (Current->db_flag)
    { if( Current->useDive)
               DiveFlush(Current);
      else
               wmFlush(Current);
    }

}

void  WMesaPaletteChange(HPAL Pal)
{
    int vRet;
#if POKA
    LPPALETTEENTRY pPal;
    if (Current && (Current->rgb_flag==GL_FALSE || Current->dither_flag == GL_TRUE))
    {
        pPal = (PALETTEENTRY *)malloc( 256 * sizeof(PALETTEENTRY));
        Current->hPal=Pal;
        //  GetPaletteEntries( Pal, 0, 256, pPal );
        GetPalette( Pal, pPal );
        vRet = SetDIBColorTable(Current->dib.hDC,0,256,pPal);
        free( pPal );
    }
#endif /* POKA */
}

void  wmSetPixel(PWMC pwc, int iScanLine, int iPixel, BYTE r, BYTE g, BYTE b)
{
 POINTL ptl;

   if(Current->db_flag)
   {
        PBYTE  lpb; /* = pwc->pbPixels; */
        UINT  *lpdw;
        unsigned short int *lpw;
        UINT    nBypp = pwc->nColorBits >>3; /* /8 */
//        UINT    nOffset = iPixel % nBypp;

        // Move the pixel buffer pointer to the scanline that we
        // want to access

        //      pwc->dib.fFlushed = FALSE;

//        lpb += pwc->ScanWidth * iScanLine;
        // Now move to the desired pixel
//        lpb += iPixel * nBypp;
        lpb = (PBYTE)PIXELADDR(iPixel, iScanLine);

        if(nBypp == 1)
        {
            if(pwc->dither_flag)
                *lpb = DITHER_RGB_2_8BIT(r,g,b,iScanLine,iPixel);
            else
                *lpb = BGR8(r,g,b);
        } else if(nBypp == 2) {
            lpw = (unsigned short int *)lpb;
            *lpw = BGR16(r,g,b);
        } else if (nBypp == 3) {
           *lpb++ = b;
           *lpb++ = g;
           *lpb   = r;
//            lpdw = (UINT  *)lpb;
//            *lpdw = BGR24(r,g,b);
        } else if (nBypp == 4) {
            lpdw = (UINT  *)lpb;
            *lpdw = BGR32(r,g,b);
        }
    } else {
       ptl.y = iScanLine;
       ptl.x = iPixel;
       GpiSetColor(pwc->hps,LONGFromRGB(r,g,b));
       GpiSetPel(pwc->hps, &ptl);
    }
}

void  wmSetPixel_db1(PWMC pwc, int iScanLine, int iPixel, BYTE r, BYTE g, BYTE b)
{
   PBYTE  lpb;
// UINT    nBypp = 1
        // Move the pixel buffer pointer to the scanline that we
        // want to access

   lpb = (PBYTE)PIXELADDR_1(iPixel, iScanLine);

    if(pwc->dither_flag)
                *lpb = DITHER_RGB_2_8BIT(r,g,b,iScanLine,iPixel);
    else
                *lpb = BGR8(r,g,b);
}

void wmSetPixel_db2(PWMC pwc, int iScanLine, int iPixel, BYTE r, BYTE g, BYTE b)
{
    unsigned short int *lpw;
//  UINT    nBypp = 2
        // Move the pixel buffer pointer to the scanline that we
        // want to access

     lpw = (unsigned short int *)PIXELADDR_2(iPixel, iScanLine);
     *lpw = BGR16(r,g,b);
}

void wmSetPixel_db3(PWMC pwc, int iScanLine, int iPixel, BYTE r, BYTE g, BYTE b)
{
//    UINT  *lpdw;
//   UINT  lpdw;
    PBYTE  lpb;
//  UINT    nBypp = 3
    lpb = ( PBYTE)PIXELADDR_3(iPixel, iScanLine);
//    lpdw = (UINT  *)PIXELADDR_3(iPixel, iScanLine);

//    lpdw = BGR24(r,g,b);

    *lpb++ = b;
    *lpb++ = g;
    *lpb   = r;

}

void wmSetPixel_db4(PWMC pwc, int iScanLine, int iPixel, BYTE r, BYTE g, BYTE b)
{
    UINT  *lpdw;
//  UINT    nBypp = 4
    lpdw = (UINT  *)PIXELADDR_4(iPixel, iScanLine);
    *lpdw = BGR32(r,g,b);
}

int wmGetPixel(PWMC pwc, int x, int y)
{
   int val;
   if(Current->db_flag)
   {
        PBYTE  lpb = pwc->pbPixels;
        UINT  *lpdw;
        unsigned short int *lpw;
        UINT    nBypp = pwc->nColorBits / 8;
//      UINT    nOffset = x % nBypp;

        // Move the pixel buffer pointer to the scanline that we
        // want to access

        //      pwc->dib.fFlushed = FALSE;

//        lpb += pwc->ScanWidth * y;
        // Now move to the desired pixel
//        lpb += x * nBypp;
      lpb = (PBYTE)PIXELADDR(x, y);


        if(nBypp == 1)
        {
           //?? if(pwc->dither_flag)
           //??      *lpb = DITHER_RGB_2_8BIT(r,g,b,iScanLine,iPixel);
           //?? else
            val = (int) (*lpb);
        } else if(nBypp == 2) {
            lpw = (unsigned short int *)lpb;
            val = (int) (*lpw);
        } else if (nBypp == 3) {
            lpdw = (UINT  *)lpb;
            val  = ((int)(*lpdw)) & 0xffffff;
        } else if (nBypp == 4) {
            lpdw = (UINT  *)lpb;
            val  =  (int) (*lpdw);
        }
    } else {
       POINTL ptl;
       ptl.y = y;
       ptl.x = x;
       val = GpiQueryPel(pwc->hps, &ptl);
    }
    return val;
}

void wmCreateDIBSection( HDC   hDC,
                         PWMC pwc,    // handle of device context
                         CONST BITMAPINFO2 *pbmi  // address of structure containing bitmap size, format, and color data
                       )
                        // ,UINT iUsage)  // color data type indicator: RGB values or palette indices
{
    int   dwSize = 0,LbmpBuff;
    int   dwScanWidth;
    UINT    nBypp = pwc->nColorBits / 8;
    HDC     hic;
   PSZ pszData[4] = { "Display", NULL, NULL, NULL };
   SIZEL sizlPage = {0, 0};
   LONG alData[2];

    dwScanWidth = (((pwc->ScanWidth * nBypp)+ 3) & ~3);

    pwc->ScanWidth =pwc->pitch = dwScanWidth;

//    if (stereo_flag)
//        pwc->ScanWidth = 2* pwc->pitch;

    dwSize = sizeof(BITMAPINFO) + (dwScanWidth * pwc->height);

    pwc->memb.bNx = (pwc->width/32+1)*32;
    pwc->memb.bNy = (pwc->height/32+1)*32;

    pwc->memb.BytesPerBmpPixel = nBypp;
    LbmpBuff = pwc->memb.BytesPerBmpPixel * pwc->memb.bNx * pwc->memb.bNy+4;

    if(pwc->memb.pBmpBuffer)
    {  if(pwc->memb.LbmpBuff != LbmpBuff)
        {   pwc->memb.pBmpBuffer = (BYTE *) realloc(pwc->memb.pBmpBuffer,LbmpBuff);
        }
    } else {
       pwc->memb.pBmpBuffer = (BYTE *)malloc(LbmpBuff);
    }
    pwc->pbPixels = pwc->memb.pBmpBuffer;
    pwc->memb.LbmpBuff = LbmpBuff;

    pwc->memb.hdcMem = DevOpenDC(pwc->hab, OD_MEMORY, "*", 4, (PDEVOPENDATA) pszData, NULLHANDLE);
    pwc->memb.hpsMem = GpiCreatePS(pwc->hab, pwc->memb.hdcMem, &sizlPage,
          PU_PELS | GPIA_ASSOC | GPIT_MICRO);


}


BYTE DITHER_RGB_2_8BIT( int red, int green, int blue, int pixel, int scanline)
{
    char unsigned redtemp, greentemp, bluetemp, paletteindex;

    //*** now, look up each value in the halftone matrix
    //*** using an 8x8 ordered dither.
    redtemp = aDividedBy51[red]
        + (aModulo51[red] > aHalftone8x8[(pixel%8)*8
        + scanline%8]);
    greentemp = aDividedBy51[(char unsigned)green]
        + (aModulo51[green] > aHalftone8x8[
        (pixel%8)*8 + scanline%8]);
    bluetemp = aDividedBy51[(char unsigned)blue]
        + (aModulo51[blue] > aHalftone8x8[
        (pixel%8)*8 +scanline%8]);

    //*** recombine the halftoned rgb values into a palette index
    paletteindex =
        redtemp + aTimes6[greentemp] + aTimes36[bluetemp];

    //*** and translate through the wing halftone palette
    //*** translation vector to give the correct value.
    return aWinGHalftoneTranslation[paletteindex];
}

//
// Blit memory DC to screen DC
//
BOOL wmFlush(PWMC pwc)
{
    BOOL  bRet = 0;
    int   dwErr = 0;
    POINTL aptl[4];
extern GLUTwindow * __glutCurrentWindow;
    HBITMAP hbmpold;
    LONG rc;
    // Now search through the torus frames and mark used colors
    if(pwc->db_flag)
    {
//      if(isChange)
      {  if(pwc->hbm)
                GpiDeleteBitmap(pwc->hbm);
         pwc->hbm = GpiCreateBitmap(pwc->memb.hpsMem, &pwc->bmi, CBM_INIT,
                                       (PBYTE)pwc->memb.pBmpBuffer, &(pwc->memb.bmi_mem));
         if(pwc->hbm == 0)
         {  printf("Error creating Bitmap\n");
         }

         hbmpold = GpiSetBitmap(pwc->memb.hpsMem,pwc->hbm);
      }
//      else
//      {    GpiSetBitmap(hpsMem,hbm);
//           GpiSetBitmapBits(hpsMem, 0,bmp.cy,(PBYTE) pBmpBuffer, pbmi);
//      }

      aptl[0].x = 0;       /* Lower-left corner of destination rectangle  */
      aptl[0].y = 0;       /* Lower-left corner of destination rectangle  */
      aptl[1].x = pwc->bmi.cx;  /* Upper-right corner of destination rectangle */
      aptl[1].y = pwc->bmi.cy; /* Upper-right corner of destination rectangle */
     /* Source-rectangle dimensions (in device coordinates)              */
      aptl[2].x = 0;      /* Lower-left corner of source rectangle       */
      aptl[2].y = 0;      /* Lower-left corner of source rectangle       */
      aptl[3].x = pwc->bmi.cx;
      aptl[3].y = pwc->bmi.cy;

     rc = GpiBitBlt(pwc->hps, pwc->memb.hpsMem,   3,   /* 4 Number of points in aptl */
         aptl, ROP_SRCCOPY,  BBO_IGNORE/* | BBO_PAL_COLORS*/ );
     if(rc = GPI_ERROR)
         {  printf("Error GpiBitBlt\n");
         }

//??            bRet = BitBlt(pwc->hDC, 0, 0, pwc->width, pwc->height,
//??                pwc->dib.hDC, 0, 0, SRCCOPY);
      GpiSetBitmap(pwc->memb.hpsMem,hbmpold);
    }



    return(TRUE);
}


static struct VideoDevConfigCaps VideoDevConfig = { 0 };

/* Получить видеоконфигуpацию по полной пpогpамме */
LONG *GetVideoConfig(HDC hdc)
{
   LONG lCount = CAPS_PHYS_COLORS;
   LONG lStart = CAPS_FAMILY; /* 0 */
   BOOL rc;

   if( hdc  || !VideoDevConfig.sts)
   {  rc = DevQueryCaps( hdc, lStart, lCount, VideoDevConfig.Caps );
      if(rc)
      {   VideoDevConfig.sts = 1;
          return VideoDevConfig.Caps;
      }
   } else {
            return VideoDevConfig.Caps;
   }
   return NULL;
}


