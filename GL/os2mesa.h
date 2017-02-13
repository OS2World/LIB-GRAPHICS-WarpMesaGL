/* os2mesa.h */

#ifndef WMESA_H
#define WMESA_H


#ifdef __cplusplus
extern "C" {
#endif


#include "gl\gl.h"

/* This is the WMesa context 'handle': */
typedef struct wmesa_context *WMesaContext;



/*
 * Create a new WMesaContext for rendering into a window.  You must
 * have already created the window of correct visual type and with an
 * appropriate colormap.
 *
 * Input:
 *         hWnd - Window handle
 *         Pal  - Palette to use
 *         rgb_flag - GL_TRUE = RGB mode,
 *                    GL_FALSE = color index mode
 *         db_flag - GL_TRUE = double-buffered,
 *                   GL_FALSE = single buffered
 *
 * Note: Indexed mode requires double buffering under Windows.
 *
 * Return:  a WMesa_context or NULL if error.
 */
extern  WMesaContext WMesaCreateContext
        ( HWND hWnd, HPAL* Pal, HPS   hpsCurrent, HAB hab,
               GLboolean rgb_flag, GLboolean db_flag, int useDive );


/*
 * Destroy a rendering context as returned by WMesaCreateContext()
 */
/*extern void WMesaDestroyContext( WMesaContext ctx );*/
extern void WMesaDestroyContext( void );


/*
 * Make the specified context the current one.
 */
extern void WMesaMakeCurrent( WMesaContext ctx );


/*
 * Return a handle to the current context.
 */
extern WMesaContext WMesaGetCurrentContext( void );


/*
 * Swap the front and back buffers for the current context.  No action
 * taken if the context is not double buffered.
 */
extern void WMesaSwapBuffers(void);


/*
 * In indexed color mode we need to know when the palette changes.
 */
extern void WMesaPaletteChange(HPAL Pal);

extern void WMesaMove(void);


HGLRC wglCreateContext(HDC hdc,HPS hpsCurrent, HAB hab);
BOOL wglCopyContext(HGLRC hglrcSrc,HGLRC hglrcDst,UINT mask);
BOOL wglMakeCurrent(HDC hdc,HGLRC hglrc);
BOOL wglSetPixelFormat(HDC hdc,int iPixelFormat,XVisualInfo *ppfd);
int  wglGetPixelFormat(HDC hdc);


#ifdef __cplusplus
}
#endif


#endif

