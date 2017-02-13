/* $Id: s_triangle.c,v 1.65 2002/11/13 16:51:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.1
 *
 * Copyright (C) 1999-2002  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/*
 * When the device driver doesn't implement triangle rasterization it
 * can hook in _swrast_Triangle, which eventually calls one of these
 * functions to draw triangles.
 */

#include "glheader.h"
#include "context.h"
#include "colormac.h"
#include "imports.h"
#include "macros.h"
#include "mmath.h"
#include "texformat.h"
#include "teximage.h"
#include "texstate.h"

#include "s_aatriangle.h"
#include "s_context.h"
#include "s_depth.h"
#include "s_feedback.h"
#include "s_span.h"
#include "s_triangle.h"

#define FIST_MAGIC ((((65536.0 * 65536.0 * 16)+(65536.0 * 0.5))* 65536.0))

/*
 * Just used for feedback mode.
 */
GLboolean _mesa_cull_triangle( GLcontext *ctx,
                           const SWvertex *v0,
                           const SWvertex *v1,
                           const SWvertex *v2 )
{
   GLfloat ex = v1->win[0] - v0->win[0];
   GLfloat ey = v1->win[1] - v0->win[1];
   GLfloat fx = v2->win[0] - v0->win[0];
   GLfloat fy = v2->win[1] - v0->win[1];
   GLfloat c = ex*fy-ey*fx;

   if (c * ((SWcontext *)ctx->swrast_context)->_backface_sign > 0)
      return 0;

   return 1;
}



/*
 * Render a flat-shaded color index triangle.
 */

/*
 * Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom triangle rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated across the triangle:
 *    INTERP_Z        - if defined, interpolate Z values
 *    INTERP_FOG      - if defined, interpolate fog values
 *    INTERP_RGB      - if defined, interpolate RGB values
 *    INTERP_ALPHA    - if defined, interpolate Alpha values (req's INTERP_RGB)
 *    INTERP_SPEC     - if defined, interpolate specular RGB values
 *    INTERP_INDEX    - if defined, interpolate color index values
 *    INTERP_INT_TEX  - if defined, interpolate integer ST texcoords
 *                         (fast, simple 2-D texture mapping)
 *    INTERP_TEX      - if defined, interpolate set 0 float STRQ texcoords
 *                         NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_MULTITEX - if defined, interpolate N units of STRQ texcoords
 *    INTERP_FLOAT_RGBA - if defined, interpolate RGBA with floating point
 *    INTERP_FLOAT_SPEC - if defined, interpolate specular with floating point
 *
 * When one can directly address pixels in the color buffer the following
 * macros can be defined and used to compute pixel addresses during
 * rasterization (see pRow):
 *    PIXEL_TYPE          - the datatype of a pixel (GLubyte, GLushort, GLuint)
 *    BYTES_PER_ROW       - number of bytes per row in the color buffer
 *    PIXEL_ADDRESS(X,Y)  - returns the address of pixel at (X,Y) where
 *                          Y==0 at bottom of screen and increases upward.
 *
 * Similarly, for direct depth buffer access, this type is used for depth
 * buffer addressing:
 *    DEPTH_TYPE          - either GLushort or GLuint
 *
 * Optionally, one may provide one-time setup code per triangle:
 *    SETUP_CODE    - code which is to be executed once per triangle
 *    CLEANUP_CODE    - code to execute at end of triangle
 *
 * The following macro MUST be defined:
 *    RENDER_SPAN(span) - code to write a span of pixels.
 *
 * This code was designed for the origin to be in the lower-left corner.
 *
 * Inspired by triangle rasterizer code written by Allen Akin.  Thanks Allen!
 */


/*
 * This is a bit of a hack, but it's a centralized place to enable floating-
 * point color interpolation when GLchan is actually floating point.
 */



static void flat_ci_triangle(GLcontext *ctx, const SWvertex *v0,
                                 const SWvertex *v1,
                                 const SWvertex *v2 )
{
   typedef struct {
        const SWvertex *v0, *v1;   /* Y(v0) < Y(v1) */
        GLfloat dx;    /* X(v1) - X(v0) */
        GLfloat dy;    /* Y(v1) - Y(v0) */
        GLfixed fdxdy; /* dx/dy in fixed-point */
        GLfixed fsx;   /* first sample point x coord */
        GLfixed fsy;
        GLfloat adjy;  /* adjust from v[0]->fy to fsy, scaled */
        GLint lines;   /* number of lines to be sampled on this edge */
        GLfixed fx0;   /* fixed pt X of lower endpoint */
   } EdgeT;

   const GLint depthBits = ctx->Visual.depthBits;
   const GLfloat maxDepth = ctx->DepthMaxF;
   EdgeT eMaj, eTop, eBot;
   GLfloat oneOverArea;
   const SWvertex *vMin, *vMid, *vMax;  /* Y(vMin)<=Y(vMid)<=Y(vMax) */
   float bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;
   const GLint snapMask = ~((0x00000800 / (1 << 4)) - 1); /* for x/y coord snapping */
   GLfixed vMin_fx, vMin_fy, vMid_fx, vMid_fy, vMax_fx, vMax_fy;

   struct sw_span span;

     (span).primitive = (0x0009);
     (span).interpMask = (0);
     (span).arrayMask = (0);
     (span).start = 0;
     (span).end = (0);
     (span).facing = 0;
     (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays;


   printf("%s()\n", __FUNCTION__);
   /*
   printf("  %g, %g, %g\n", v0->win[0], v0->win[1], v0->win[2]);
   printf("  %g, %g, %g\n", v1->win[0], v1->win[1], v1->win[2]);
   printf("  %g, %g, %g\n", v2->win[0], v2->win[1], v2->win[2]);
    */

   /* Compute fixed point x,y coords w/ half-pixel offsets and snapping.
    * And find the order of the 3 vertices along the Y axis.
    */
   {
      const GLfixed fy0 = FloatToFixed(v0->win[1] - 0.5F) & snapMask;
      const GLfixed fy1 = FloatToFixed(v1->win[1] - 0.5F) & snapMask;
      const GLfixed fy2 = FloatToFixed(v2->win[1] - 0.5F) & snapMask;

      if (fy0 <= fy1) {
         if (fy1 <= fy2) {
            /* y0 <= y1 <= y2 */
            vMin = v0;   vMid = v1;   vMax = v2;
            vMin_fy = fy0;  vMid_fy = fy1;  vMax_fy = fy2;
         }
         else if (fy2 <= fy0) {
            /* y2 <= y0 <= y1 */
            vMin = v2;   vMid = v0;   vMax = v1;
            vMin_fy = fy2;  vMid_fy = fy0;  vMax_fy = fy1;
         }
         else {
            /* y0 <= y2 <= y1 */
            vMin = v0;   vMid = v2;   vMax = v1;
            vMin_fy = fy0;  vMid_fy = fy2;  vMax_fy = fy1;
            bf = -bf;
         }
      }
      else {
         if (fy0 <= fy2) {
            /* y1 <= y0 <= y2 */
            vMin = v1;   vMid = v0;   vMax = v2;
            vMin_fy = fy1;  vMid_fy = fy0;  vMax_fy = fy2;
            bf = -bf;
         }
         else if (fy2 <= fy1) {
            /* y2 <= y1 <= y0 */
            vMin = v2;   vMid = v1;   vMax = v0;
            vMin_fy = fy2;  vMid_fy = fy1;  vMax_fy = fy0;
            bf = -bf;
         }
         else {
            /* y1 <= y2 <= y0 */
            vMin = v1;   vMid = v2;   vMax = v0;
            vMin_fy = fy1;  vMid_fy = fy2;  vMax_fy = fy0;
         }
      }

      /* fixed point X coords */
      vMin_fx = (((int) ((((vMin->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMin->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMin->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMid_fx = (((int) ((((vMid->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMid->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMid->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMax_fx = (((int) ((((vMax->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMax->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMax->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
   }

   /* vertex/edge relationship */
   eMaj.v0 = vMin;   eMaj.v1 = vMax;   /*TODO: .v1's not needed */
   eTop.v0 = vMid;   eTop.v1 = vMax;
   eBot.v0 = vMin;   eBot.v1 = vMid;

   /* compute deltas for each edge:  vertex[upper] - vertex[lower] */
   eMaj.dx = ((vMax_fx - vMin_fx) * (1.0F / 2048.0f));
   eMaj.dy = ((vMax_fy - vMin_fy) * (1.0F / 2048.0f));
   eTop.dx = ((vMax_fx - vMid_fx) * (1.0F / 2048.0f));
   eTop.dy = ((vMax_fy - vMid_fy) * (1.0F / 2048.0f));
   eBot.dx = ((vMid_fx - vMin_fx) * (1.0F / 2048.0f));
   eBot.dy = ((vMid_fy - vMin_fy) * (1.0F / 2048.0f));

   /* compute area, oneOverArea and perform backface culling */
   {
      const GLfloat area = eMaj.dx * eBot.dy - eBot.dx * eMaj.dy;

      /* Do backface culling */
      if (area * bf < 0.0)
         return;

      if (IS_INF_OR_NAN(area) || area == 0.0F)
         return;

      oneOverArea = 1.0F / area;
   }

   ctx->OcclusionResult = 0x1;
   span.facing = ctx->_Facing; /* for 2-sided stencil test */

   /* Edge setup.  For a triangle strip these could be reused... */
   {
      eMaj.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eMaj.lines = (((((vMax_fy - eMaj.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eMaj.lines > 0) {
         GLfloat dxdy = eMaj.dx / eMaj.dy;
         eMaj.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eMaj.adjy = (GLfloat) (eMaj.fsy - vMin_fy);  /* SCALED! */
         eMaj.fx0 = vMin_fx;
         eMaj.fsx = eMaj.fx0 + (GLfixed) (eMaj.adjy * dxdy);
      }
      else {
         return;  /*CULLED*/
      }

      eTop.fsy = (((vMid_fy) + 0x00000800 - 1) & (~0x000007FF));
      eTop.lines = (((((vMax_fy - eTop.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eTop.lines > 0) {
         GLfloat dxdy = eTop.dx / eTop.dy;
         eTop.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eTop.adjy = (GLfloat) (eTop.fsy - vMid_fy); /* SCALED! */
         eTop.fx0 = vMid_fx;
         eTop.fsx = eTop.fx0 + (GLfixed) (eTop.adjy * dxdy);
      }

      eBot.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eBot.lines = (((((vMid_fy - eBot.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eBot.lines > 0) {
         GLfloat dxdy = eBot.dx / eBot.dy;
         eBot.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eBot.adjy = (GLfloat) (eBot.fsy - vMin_fy);  /* SCALED! */
         eBot.fx0 = vMin_fx;
         eBot.fsx = eBot.fx0 + (GLfixed) (eBot.adjy * dxdy);
      }
   }

   /*
    * Conceptually, we view a triangle as two subtriangles
    * separated by a perfectly horizontal line.  The edge that is
    * intersected by this line is one with maximal absolute dy; we
    * call it a ``major'' edge.  The other two edges are the
    * ``top'' edge (for the upper subtriangle) and the ``bottom''
    * edge (for the lower subtriangle).  If either of these two
    * edges is horizontal or very close to horizontal, the
    * corresponding subtriangle might cover zero sample points;
    * we take care to handle such cases, for performance as well
    * as correctness.
    *
    * By stepping rasterization parameters along the major edge,
    * we can avoid recomputing them at the discontinuity where
    * the top and bottom edges meet.  However, this forces us to
    * be able to scan both left-to-right and right-to-left.
    * Also, we must determine whether the major edge is at the
    * left or right side of the triangle.  We do this by
    * computing the magnitude of the cross-product of the major
    * and top edges.  Since this magnitude depends on the sine of
    * the angle between the two edges, its sign tells us whether
    * we turn to the left or to the right when travelling along
    * the major edge to the top edge, and from this we infer
    * whether the major edge is on the left or the right.
    *
    * Serendipitously, this cross-product magnitude is also a
    * value we need to compute the iteration parameter
    * derivatives for the triangle, and it can be used to perform
    * backface culling because its sign tells us whether the
    * triangle is clockwise or counterclockwise.  In this code we
    * refer to it as ``area'' because it's also proportional to
    * the pixel area of the triangle.
    */

   {
      GLint scan_from_left_to_right;  /* true if scanning left-to-right */
      GLfloat dzdx, dzdy;
      GLfloat dfogdy;

      /*
       * Execute user-supplied setup code
       */
      span.interpMask |= 0x004; span.index = ((v2->index) << 11); span.indexStep = 0;

      scan_from_left_to_right = (oneOverArea < 0.0F);


      /* compute d?/dx and d?/dy derivatives */
      span.interpMask |= 0x008;
      {
         GLfloat eMaj_dz, eBot_dz;
         eMaj_dz = vMax->win[2] - vMin->win[2];
         eBot_dz = vMid->win[2] - vMin->win[2];
         dzdx = oneOverArea * (eMaj_dz * eBot.dy - eMaj.dy * eBot_dz);
         if (dzdx > maxDepth || dzdx < -maxDepth) {
            /* probably a sliver triangle */
            dzdx = 0.0;
            dzdy = 0.0;
         }
         else {
            dzdy = oneOverArea * (eMaj.dx * eBot_dz - eMaj_dz * eBot.dx);
         }
         if (depthBits <= 16)
            span.zStep = (((int) ((((dzdx) * 2048.0f) >= 0.0F) ? (((dzdx) * 2048.0f) + 0.5F) : (((dzdx) * 2048.0f) - 0.5F))));
         else
            span.zStep = (GLint) dzdx;
      }
      span.interpMask |= 0x010;
      {
         const GLfloat eMaj_dfog = vMax->fog - vMin->fog;
         const GLfloat eBot_dfog = vMid->fog - vMin->fog;
         span.fogStep = oneOverArea * (eMaj_dfog * eBot.dy - eMaj.dy * eBot_dfog);
         dfogdy = oneOverArea * (eMaj.dx * eBot_dfog - eMaj_dfog * eBot.dx);
      }

      /*
       * We always sample at pixel centers.  However, we avoid
       * explicit half-pixel offsets in this code by incorporating
       * the proper offset in each of x and y during the
       * transformation to window coordinates.
       *
       * We also apply the usual rasterization rules to prevent
       * cracks and overlaps.  A pixel is considered inside a
       * subtriangle if it meets all of four conditions: it is on or
       * to the right of the left edge, strictly to the left of the
       * right edge, on or below the top edge, and strictly above
       * the bottom edge.  (Some edges may be degenerate.)
       *
       * The following discussion assumes left-to-right scanning
       * (that is, the major edge is on the left); the right-to-left
       * case is a straightforward variation.
       *
       * We start by finding the half-integral y coordinate that is
       * at or below the top of the triangle.  This gives us the
       * first scan line that could possibly contain pixels that are
       * inside the triangle.
       *
       * Next we creep down the major edge until we reach that y,
       * and compute the corresponding x coordinate on the edge.
       * Then we find the half-integral x that lies on or just
       * inside the edge.  This is the first pixel that might lie in
       * the interior of the triangle.  (We won't know for sure
       * until we check the other edges.)
       *
       * As we rasterize the triangle, we'll step down the major
       * edge.  For each step in y, we'll move an integer number
       * of steps in x.  There are two possible x step sizes, which
       * we'll call the ``inner'' step (guaranteed to land on the
       * edge or inside it) and the ``outer'' step (guaranteed to
       * land on the edge or outside it).  The inner and outer steps
       * differ by one.  During rasterization we maintain an error
       * term that indicates our distance from the true edge, and
       * select either the inner step or the outer step, whichever
       * gets us to the first pixel that falls inside the triangle.
       *
       * All parameters (z, red, etc.) as well as the buffer
       * addresses for color and z have inner and outer step values,
       * so that we can increment them appropriately.  This method
       * eliminates the need to adjust parameters by creeping a
       * sub-pixel amount into the triangle at each scanline.
       */

      {
         int subTriangle;
         GLfixed fx;
         GLfixed fxLeftEdge = 0, fxRightEdge = 0;
         GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
         GLfixed fdxOuter;
         int idxOuter;
         float dxOuter;
         GLfixed fError = 0, fdError = 0;
         float adjx, adjy;
         GLfixed fy;
         GLfixed fz = 0, fdzOuter = 0, fdzInner;
         GLfloat fogLeft = 0, dfogOuter = 0, dfogInner;

         for (subTriangle=0; subTriangle<=1; subTriangle++) {
            EdgeT *eLeft, *eRight;
            int setupLeft, setupRight;
            int lines;

            if (subTriangle==0) {
               /* bottom half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eBot;
                  lines = eRight->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
               else {
                  eLeft = &eBot;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
            }
            else {
               /* top half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eTop;
                  lines = eRight->lines;
                  setupLeft = 0;
                  setupRight = 1;
               }
               else {
                  eLeft = &eTop;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 0;
               }
               if (lines == 0)
                  return;
            }

            if (setupLeft && eLeft->lines > 0) {
               const SWvertex *vLower;
               GLfixed fsx = eLeft->fsx;
               fx = (((fsx) + 0x00000800 - 1) & (~0x000007FF));
               fError = fx - fsx - 0x00000800;
               fxLeftEdge = fsx - 1;
               fdxLeftEdge = eLeft->fdxdy;
               fdxOuter = ((fdxLeftEdge - 1) & (~0x000007FF));
               fdError = fdxOuter - fdxLeftEdge + 0x00000800;
               idxOuter = ((fdxOuter) >> 11);
               dxOuter = (float) idxOuter;
               (void) dxOuter;

               fy = eLeft->fsy;
               span.y = ((fy) >> 11);

               adjx = (float)(fx - eLeft->fx0);  /* SCALED! */
               adjy = eLeft->adjy;              /* SCALED! */
               vLower = eLeft->v0;

               /*
                * Now we need the set of parameter (z, color, etc.) values at
                * the point (fx, fy).  This gives us properly-sampled parameter
                * values that we can step from pixel to pixel.  Furthermore,
                * although we might have intermediate results that overflow
                * the normal parameter range when we step temporarily outside
                * the triangle, we shouldn't overflow or underflow for any
                * pixel that's actually inside the triangle.
                */

               {
                  GLfloat z0 = vLower->win[2];
                  if (depthBits <= 16) {
                     /* interpolate fixed-pt values */
                     GLfloat tmp = (z0 * 2048.0f +
                                    dzdx * adjx + dzdy * adjy) + 0x00000400;
                     if (tmp < 0xffffffff / 2)
                        fz = (GLfixed) tmp;
                     else
                        fz = 0xffffffff / 2;
                     fdzOuter = (((int) ((((dzdy + dxOuter * dzdx) * 2048.0f) >= 0.0F) ? (((dzdy + dxOuter * dzdx) * 2048.0f) + 0.5F) : (((dzdy + dxOuter * dzdx) * 2048.0f) - 0.5F))));
                  }
                  else {
                     /* interpolate depth values exactly */
                     fz = (GLint) (z0 + dzdx * ((adjx) * (1.0F / 2048.0f))
                                   + dzdy * ((adjy) * (1.0F / 2048.0f)));
                     fdzOuter = (GLint) (dzdy + dxOuter * dzdx);
                  }
               }
               fogLeft = vLower->fog + (span.fogStep * adjx + dfogdy * adjy)
                                       * (1.0F/2048.0f);
               dfogOuter = dfogdy + dxOuter * span.fogStep;

            } /*if setupLeft*/


            if (setupRight && eRight->lines>0) {
               fxRightEdge = eRight->fsx - 1;
               fdxRightEdge = eRight->fdxdy;
            }

            if (lines==0) {
               continue;
            }


            /* Rasterize setup */
            fdzInner = fdzOuter + span.zStep;
            dfogInner = dfogOuter + span.fogStep;

            while (lines > 0) {
               /* initialize the span interpolants to the leftmost value */
               /* ff = fixed-pt fragment */
               const GLint right = ((fxRightEdge) >> 11);

               span.x = ((fxLeftEdge) >> 11);

               if (right <= span.x)
                  span.end = 0;
               else
                  span.end = right - span.x;

               span.z = fz;
               span.fog = fogLeft;




               /* This is where we actually generate fragments */
               if (span.end > 0) {
                  _mesa_write_index_span(ctx, &span);;
               }

               /*
                * Advance to the next scan line.  Compute the
                * new edge coordinates, and adjust the
                * pixel-center x coordinate so that it stays
                * on or inside the major edge.
                */
               (span.y)++;
               lines--;

               fxLeftEdge += fdxLeftEdge;
               fxRightEdge += fdxRightEdge;


               fError += fdError;
               if (fError >= 0) {
                  fError -= 0x00000800;
                  fz += fdzOuter;
                  fogLeft += dfogOuter;
               }
               else {
                  fz += fdzInner;
                  fogLeft += dfogInner;
               }
            } /*while lines>0*/

         } /* for subTriangle */

      }
   }
}


/*
 * Render a smooth-shaded color index triangle.
 */
/*
 * Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom triangle rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated across the triangle:
 *    INTERP_Z        - if defined, interpolate Z values
 *    INTERP_FOG      - if defined, interpolate fog values
 *    INTERP_RGB      - if defined, interpolate RGB values
 *    INTERP_ALPHA    - if defined, interpolate Alpha values (req's INTERP_RGB)
 *    INTERP_SPEC     - if defined, interpolate specular RGB values
 *    INTERP_INDEX    - if defined, interpolate color index values
 *    INTERP_INT_TEX  - if defined, interpolate integer ST texcoords
 *                         (fast, simple 2-D texture mapping)
 *    INTERP_TEX      - if defined, interpolate set 0 float STRQ texcoords
 *                         NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_MULTITEX - if defined, interpolate N units of STRQ texcoords
 *    INTERP_FLOAT_RGBA - if defined, interpolate RGBA with floating point
 *    INTERP_FLOAT_SPEC - if defined, interpolate specular with floating point
 *
 * When one can directly address pixels in the color buffer the following
 * macros can be defined and used to compute pixel addresses during
 * rasterization (see pRow):
 *    PIXEL_TYPE          - the datatype of a pixel (GLubyte, GLushort, GLuint)
 *    BYTES_PER_ROW       - number of bytes per row in the color buffer
 *    PIXEL_ADDRESS(X,Y)  - returns the address of pixel at (X,Y) where
 *                          Y==0 at bottom of screen and increases upward.
 *
 * Similarly, for direct depth buffer access, this type is used for depth
 * buffer addressing:
 *    DEPTH_TYPE          - either GLushort or GLuint
 *
 * Optionally, one may provide one-time setup code per triangle:
 *    SETUP_CODE    - code which is to be executed once per triangle
 *    CLEANUP_CODE    - code to execute at end of triangle
 *
 * The following macro MUST be defined:
 *    RENDER_SPAN(span) - code to write a span of pixels.
 *
 * This code was designed for the origin to be in the lower-left corner.
 *
 * Inspired by triangle rasterizer code written by Allen Akin.  Thanks Allen!
 */


/*
 * This is a bit of a hack, but it's a centralized place to enable floating-
 * point color interpolation when GLchan is actually floating point.
 */



static void smooth_ci_triangle(GLcontext *ctx, const SWvertex *v0,
                                 const SWvertex *v1,
                                 const SWvertex *v2 )
{
   typedef struct {
        const SWvertex *v0, *v1;   /* Y(v0) < Y(v1) */
        GLfloat dx;    /* X(v1) - X(v0) */
        GLfloat dy;    /* Y(v1) - Y(v0) */
        GLfixed fdxdy; /* dx/dy in fixed-point */
        GLfixed fsx;   /* first sample point x coord */
        GLfixed fsy;
        GLfloat adjy;  /* adjust from v[0]->fy to fsy, scaled */
        GLint lines;   /* number of lines to be sampled on this edge */
        GLfixed fx0;   /* fixed pt X of lower endpoint */
   } EdgeT;

   const GLint depthBits = ctx->Visual.depthBits;
   const GLfloat maxDepth = ctx->DepthMaxF;
   EdgeT eMaj, eTop, eBot;
   GLfloat oneOverArea;
   const SWvertex *vMin, *vMid, *vMax;  /* Y(vMin)<=Y(vMid)<=Y(vMax) */
   float bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;
   const GLint snapMask = ~((0x00000800 / (1 << 4)) - 1); /* for x/y coord snapping */
   GLfixed vMin_fx, vMin_fy, vMid_fx, vMid_fy, vMax_fx, vMax_fy;

   struct sw_span span;

   (span).primitive = (0x0009);
   (span).interpMask = (0);
   (span).arrayMask = (0);
   (span).start = 0;
   (span).end = (0);
   (span).facing = 0;
   (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays;


   printf("%s()\n", __FUNCTION__);
   /*
   printf("  %g, %g, %g\n", v0->win[0], v0->win[1], v0->win[2]);
   printf("  %g, %g, %g\n", v1->win[0], v1->win[1], v1->win[2]);
   printf("  %g, %g, %g\n", v2->win[0], v2->win[1], v2->win[2]);
    */

   /* Compute fixed point x,y coords w/ half-pixel offsets and snapping.
    * And find the order of the 3 vertices along the Y axis.
    */
   {
      const GLfixed fy0 = FloatToFixed(v0->win[1] - 0.5F) & snapMask;
      const GLfixed fy1 = FloatToFixed(v1->win[1] - 0.5F) & snapMask;
      const GLfixed fy2 = FloatToFixed(v2->win[1] - 0.5F) & snapMask;

      if (fy0 <= fy1) {
         if (fy1 <= fy2) {
            /* y0 <= y1 <= y2 */
            vMin = v0;   vMid = v1;   vMax = v2;
            vMin_fy = fy0;  vMid_fy = fy1;  vMax_fy = fy2;
         }
         else if (fy2 <= fy0) {
            /* y2 <= y0 <= y1 */
            vMin = v2;   vMid = v0;   vMax = v1;
            vMin_fy = fy2;  vMid_fy = fy0;  vMax_fy = fy1;
         }
         else {
            /* y0 <= y2 <= y1 */
            vMin = v0;   vMid = v2;   vMax = v1;
            vMin_fy = fy0;  vMid_fy = fy2;  vMax_fy = fy1;
            bf = -bf;
         }
      }
      else {
         if (fy0 <= fy2) {
            /* y1 <= y0 <= y2 */
            vMin = v1;   vMid = v0;   vMax = v2;
            vMin_fy = fy1;  vMid_fy = fy0;  vMax_fy = fy2;
            bf = -bf;
         }
         else if (fy2 <= fy1) {
            /* y2 <= y1 <= y0 */
            vMin = v2;   vMid = v1;   vMax = v0;
            vMin_fy = fy2;  vMid_fy = fy1;  vMax_fy = fy0;
            bf = -bf;
         }
         else {
            /* y1 <= y2 <= y0 */
            vMin = v1;   vMid = v2;   vMax = v0;
            vMin_fy = fy1;  vMid_fy = fy2;  vMax_fy = fy0;
         }
      }

      /* fixed point X coords */
      vMin_fx = (((int) ((((vMin->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMin->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMin->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMid_fx = (((int) ((((vMid->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMid->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMid->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMax_fx = (((int) ((((vMax->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMax->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMax->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
   }

   /* vertex/edge relationship */
   eMaj.v0 = vMin;   eMaj.v1 = vMax;   /*TODO: .v1's not needed */
   eTop.v0 = vMid;   eTop.v1 = vMax;
   eBot.v0 = vMin;   eBot.v1 = vMid;

   /* compute deltas for each edge:  vertex[upper] - vertex[lower] */
   eMaj.dx = ((vMax_fx - vMin_fx) * (1.0F / 2048.0f));
   eMaj.dy = ((vMax_fy - vMin_fy) * (1.0F / 2048.0f));
   eTop.dx = ((vMax_fx - vMid_fx) * (1.0F / 2048.0f));
   eTop.dy = ((vMax_fy - vMid_fy) * (1.0F / 2048.0f));
   eBot.dx = ((vMid_fx - vMin_fx) * (1.0F / 2048.0f));
   eBot.dy = ((vMid_fy - vMin_fy) * (1.0F / 2048.0f));

   /* compute area, oneOverArea and perform backface culling */
   {
      const GLfloat area = eMaj.dx * eBot.dy - eBot.dx * eMaj.dy;

      /* Do backface culling */
      if (area * bf < 0.0)
         return;

      if (IS_INF_OR_NAN(area) || area == 0.0F)
         return;

      oneOverArea = 1.0F / area;
   }

   ctx->OcclusionResult = 0x1;
   span.facing = ctx->_Facing; /* for 2-sided stencil test */

   /* Edge setup.  For a triangle strip these could be reused... */
   {
      eMaj.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eMaj.lines = (((((vMax_fy - eMaj.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eMaj.lines > 0) {
         GLfloat dxdy = eMaj.dx / eMaj.dy;
         eMaj.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eMaj.adjy = (GLfloat) (eMaj.fsy - vMin_fy);  /* SCALED! */
         eMaj.fx0 = vMin_fx;
         eMaj.fsx = eMaj.fx0 + (GLfixed) (eMaj.adjy * dxdy);
      }
      else {
         return;  /*CULLED*/
      }

      eTop.fsy = (((vMid_fy) + 0x00000800 - 1) & (~0x000007FF));
      eTop.lines = (((((vMax_fy - eTop.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eTop.lines > 0) {
         GLfloat dxdy = eTop.dx / eTop.dy;
         eTop.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eTop.adjy = (GLfloat) (eTop.fsy - vMid_fy); /* SCALED! */
         eTop.fx0 = vMid_fx;
         eTop.fsx = eTop.fx0 + (GLfixed) (eTop.adjy * dxdy);
      }

      eBot.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eBot.lines = (((((vMid_fy - eBot.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eBot.lines > 0) {
         GLfloat dxdy = eBot.dx / eBot.dy;
         eBot.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eBot.adjy = (GLfloat) (eBot.fsy - vMin_fy);  /* SCALED! */
         eBot.fx0 = vMin_fx;
         eBot.fsx = eBot.fx0 + (GLfixed) (eBot.adjy * dxdy);
      }
   }

   /*
    * Conceptually, we view a triangle as two subtriangles
    * separated by a perfectly horizontal line.  The edge that is
    * intersected by this line is one with maximal absolute dy; we
    * call it a ``major'' edge.  The other two edges are the
    * ``top'' edge (for the upper subtriangle) and the ``bottom''
    * edge (for the lower subtriangle).  If either of these two
    * edges is horizontal or very close to horizontal, the
    * corresponding subtriangle might cover zero sample points;
    * we take care to handle such cases, for performance as well
    * as correctness.
    *
    * By stepping rasterization parameters along the major edge,
    * we can avoid recomputing them at the discontinuity where
    * the top and bottom edges meet.  However, this forces us to
    * be able to scan both left-to-right and right-to-left.
    * Also, we must determine whether the major edge is at the
    * left or right side of the triangle.  We do this by
    * computing the magnitude of the cross-product of the major
    * and top edges.  Since this magnitude depends on the sine of
    * the angle between the two edges, its sign tells us whether
    * we turn to the left or to the right when travelling along
    * the major edge to the top edge, and from this we infer
    * whether the major edge is on the left or the right.
    *
    * Serendipitously, this cross-product magnitude is also a
    * value we need to compute the iteration parameter
    * derivatives for the triangle, and it can be used to perform
    * backface culling because its sign tells us whether the
    * triangle is clockwise or counterclockwise.  In this code we
    * refer to it as ``area'' because it's also proportional to
    * the pixel area of the triangle.
    */

   {
      GLint scan_from_left_to_right;  /* true if scanning left-to-right */
      GLfloat dzdx, dzdy;
      GLfloat dfogdy;
      GLfloat didx, didy;

      /*
       * Execute user-supplied setup code
       */

      scan_from_left_to_right = (oneOverArea < 0.0F);


      /* compute d?/dx and d?/dy derivatives */
      span.interpMask |= 0x008;
      {
         GLfloat eMaj_dz, eBot_dz;
         eMaj_dz = vMax->win[2] - vMin->win[2];
         eBot_dz = vMid->win[2] - vMin->win[2];
         dzdx = oneOverArea * (eMaj_dz * eBot.dy - eMaj.dy * eBot_dz);
         if (dzdx > maxDepth || dzdx < -maxDepth) {
            /* probably a sliver triangle */
            dzdx = 0.0;
            dzdy = 0.0;
         }
         else {
            dzdy = oneOverArea * (eMaj.dx * eBot_dz - eMaj_dz * eBot.dx);
         }
         if (depthBits <= 16)
            span.zStep = (((int) ((((dzdx) * 2048.0f) >= 0.0F) ? (((dzdx) * 2048.0f) + 0.5F) : (((dzdx) * 2048.0f) - 0.5F))));
         else
            span.zStep = (GLint) dzdx;
      }
      span.interpMask |= 0x010;
      {
         const GLfloat eMaj_dfog = vMax->fog - vMin->fog;
         const GLfloat eBot_dfog = vMid->fog - vMin->fog;
         span.fogStep = oneOverArea * (eMaj_dfog * eBot.dy - eMaj.dy * eBot_dfog);
         dfogdy = oneOverArea * (eMaj.dx * eBot_dfog - eMaj_dfog * eBot.dx);
      }
      span.interpMask |= 0x004;
      if (ctx->Light.ShadeModel == 0x1D01) {
         GLfloat eMaj_di, eBot_di;
         eMaj_di = (GLfloat) ((GLint) vMax->index - (GLint) vMin->index);
         eBot_di = (GLfloat) ((GLint) vMid->index - (GLint) vMin->index);
         didx = oneOverArea * (eMaj_di * eBot.dy - eMaj.dy * eBot_di);
         span.indexStep = (((int) ((((didx) * 2048.0f) >= 0.0F) ? (((didx) * 2048.0f) + 0.5F) : (((didx) * 2048.0f) - 0.5F))));
         didy = oneOverArea * (eMaj.dx * eBot_di - eMaj_di * eBot.dx);
      }
      else {
         span.interpMask |= 0x200;
         didx = didy = 0.0F;
         span.indexStep = 0;
      }

      /*
       * We always sample at pixel centers.  However, we avoid
       * explicit half-pixel offsets in this code by incorporating
       * the proper offset in each of x and y during the
       * transformation to window coordinates.
       *
       * We also apply the usual rasterization rules to prevent
       * cracks and overlaps.  A pixel is considered inside a
       * subtriangle if it meets all of four conditions: it is on or
       * to the right of the left edge, strictly to the left of the
       * right edge, on or below the top edge, and strictly above
       * the bottom edge.  (Some edges may be degenerate.)
       *
       * The following discussion assumes left-to-right scanning
       * (that is, the major edge is on the left); the right-to-left
       * case is a straightforward variation.
       *
       * We start by finding the half-integral y coordinate that is
       * at or below the top of the triangle.  This gives us the
       * first scan line that could possibly contain pixels that are
       * inside the triangle.
       *
       * Next we creep down the major edge until we reach that y,
       * and compute the corresponding x coordinate on the edge.
       * Then we find the half-integral x that lies on or just
       * inside the edge.  This is the first pixel that might lie in
       * the interior of the triangle.  (We won't know for sure
       * until we check the other edges.)
       *
       * As we rasterize the triangle, we'll step down the major
       * edge.  For each step in y, we'll move an integer number
       * of steps in x.  There are two possible x step sizes, which
       * we'll call the ``inner'' step (guaranteed to land on the
       * edge or inside it) and the ``outer'' step (guaranteed to
       * land on the edge or outside it).  The inner and outer steps
       * differ by one.  During rasterization we maintain an error
       * term that indicates our distance from the true edge, and
       * select either the inner step or the outer step, whichever
       * gets us to the first pixel that falls inside the triangle.
       *
       * All parameters (z, red, etc.) as well as the buffer
       * addresses for color and z have inner and outer step values,
       * so that we can increment them appropriately.  This method
       * eliminates the need to adjust parameters by creeping a
       * sub-pixel amount into the triangle at each scanline.
       */

      {
         int subTriangle;
         GLfixed fx;
         GLfixed fxLeftEdge = 0, fxRightEdge = 0;
         GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
         GLfixed fdxOuter;
         int idxOuter;
         float dxOuter;
         GLfixed fError = 0, fdError = 0;
         float adjx, adjy;
         GLfixed fy;
         GLfixed fz = 0, fdzOuter = 0, fdzInner;
         GLfloat fogLeft = 0, dfogOuter = 0, dfogInner;
         GLfixed fi=0, fdiOuter=0, fdiInner;

         for (subTriangle=0; subTriangle<=1; subTriangle++) {
            EdgeT *eLeft, *eRight;
            int setupLeft, setupRight;
            int lines;

            if (subTriangle==0) {
               /* bottom half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eBot;
                  lines = eRight->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
               else {
                  eLeft = &eBot;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
            }
            else {
               /* top half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eTop;
                  lines = eRight->lines;
                  setupLeft = 0;
                  setupRight = 1;
               }
               else {
                  eLeft = &eTop;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 0;
               }
               if (lines == 0)
                  return;
            }

            if (setupLeft && eLeft->lines > 0) {
               const SWvertex *vLower;
               GLfixed fsx = eLeft->fsx;
               fx = (((fsx) + 0x00000800 - 1) & (~0x000007FF));
               fError = fx - fsx - 0x00000800;
               fxLeftEdge = fsx - 1;
               fdxLeftEdge = eLeft->fdxdy;
               fdxOuter = ((fdxLeftEdge - 1) & (~0x000007FF));
               fdError = fdxOuter - fdxLeftEdge + 0x00000800;
               idxOuter = ((fdxOuter) >> 11);
               dxOuter = (float) idxOuter;
               (void) dxOuter;

               fy = eLeft->fsy;
               span.y = ((fy) >> 11);

               adjx = (float)(fx - eLeft->fx0);  /* SCALED! */
               adjy = eLeft->adjy;              /* SCALED! */
               vLower = eLeft->v0;

               /*
                * Now we need the set of parameter (z, color, etc.) values at
                * the point (fx, fy).  This gives us properly-sampled parameter
                * values that we can step from pixel to pixel.  Furthermore,
                * although we might have intermediate results that overflow
                * the normal parameter range when we step temporarily outside
                * the triangle, we shouldn't overflow or underflow for any
                * pixel that's actually inside the triangle.
                */

               {
                  GLfloat z0 = vLower->win[2];
                  if (depthBits <= 16) {
                     /* interpolate fixed-pt values */
                     GLfloat tmp = (z0 * 2048.0f +
                                    dzdx * adjx + dzdy * adjy) + 0x00000400;
                     if (tmp < 0xffffffff / 2)
                        fz = (GLfixed) tmp;
                     else
                        fz = 0xffffffff / 2;
                     fdzOuter = (((int) ((((dzdy + dxOuter * dzdx) * 2048.0f) >= 0.0F) ? (((dzdy + dxOuter * dzdx) * 2048.0f) + 0.5F) : (((dzdy + dxOuter * dzdx) * 2048.0f) - 0.5F))));
                  }
                  else {
                     /* interpolate depth values exactly */
                     fz = (GLint) (z0 + dzdx * ((adjx) * (1.0F / 2048.0f))
                                   + dzdy * ((adjy) * (1.0F / 2048.0f)));
                     fdzOuter = (GLint) (dzdy + dxOuter * dzdx);
                  }
               }
               fogLeft = vLower->fog + (span.fogStep * adjx + dfogdy * adjy)
                                       * (1.0F/2048.0f);
               dfogOuter = dfogdy + dxOuter * span.fogStep;
               if (ctx->Light.ShadeModel == 0x1D01) {
                  fi = (GLfixed)(vLower->index * 2048.0f
                                 + didx * adjx + didy * adjy) + 0x00000400;
                  fdiOuter = (((int) ((((didy + dxOuter * didx) * 2048.0f) >= 0.0F) ? (((didy + dxOuter * didx) * 2048.0f) + 0.5F) : (((didy + dxOuter * didx) * 2048.0f) - 0.5F))));
               }
               else {
                  fi = (GLfixed) (v2->index * 2048.0f);
                  fdiOuter = 0;
               }

            } /*if setupLeft*/


            if (setupRight && eRight->lines>0) {
               fxRightEdge = eRight->fsx - 1;
               fdxRightEdge = eRight->fdxdy;
            }

            if (lines==0) {
               continue;
            }


            /* Rasterize setup */
            fdzInner = fdzOuter + span.zStep;
            dfogInner = dfogOuter + span.fogStep;
            fdiInner = fdiOuter + span.indexStep;

            while (lines > 0) {
               /* initialize the span interpolants to the leftmost value */
               /* ff = fixed-pt fragment */
               const GLint right = ((fxRightEdge) >> 11);

               span.x = ((fxLeftEdge) >> 11);

               if (right <= span.x)
                  span.end = 0;
               else
                  span.end = right - span.x;

               span.z = fz;
               span.fog = fogLeft;
               span.index = fi;



               if (span.index < 0)  span.index = 0;

               /* This is where we actually generate fragments */
               if (span.end > 0) {
                  _mesa_write_index_span(ctx, &span);;
               }

               /*
                * Advance to the next scan line.  Compute the
                * new edge coordinates, and adjust the
                * pixel-center x coordinate so that it stays
                * on or inside the major edge.
                */
               (span.y)++;
               lines--;

               fxLeftEdge += fdxLeftEdge;
               fxRightEdge += fdxRightEdge;


               fError += fdError;
               if (fError >= 0) {
                  fError -= 0x00000800;
                  fz += fdzOuter;
                  fogLeft += dfogOuter;
                  fi += fdiOuter;
               }
               else {
                  fz += fdzInner;
                  fogLeft += dfogInner;
                  fi += fdiInner;
               }
            } /*while lines>0*/

         } /* for subTriangle */

      }
   }
}

/*
 * Render a flat-shaded RGBA triangle.
 */

/*
 * Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom triangle rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated across the triangle:
 *    INTERP_Z        - if defined, interpolate Z values
 *    INTERP_FOG      - if defined, interpolate fog values
 *    INTERP_RGB      - if defined, interpolate RGB values
 *    INTERP_ALPHA    - if defined, interpolate Alpha values (req's INTERP_RGB)
 *    INTERP_SPEC     - if defined, interpolate specular RGB values
 *    INTERP_INDEX    - if defined, interpolate color index values
 *    INTERP_INT_TEX  - if defined, interpolate integer ST texcoords
 *                         (fast, simple 2-D texture mapping)
 *    INTERP_TEX      - if defined, interpolate set 0 float STRQ texcoords
 *                         NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_MULTITEX - if defined, interpolate N units of STRQ texcoords
 *    INTERP_FLOAT_RGBA - if defined, interpolate RGBA with floating point
 *    INTERP_FLOAT_SPEC - if defined, interpolate specular with floating point
 *
 * When one can directly address pixels in the color buffer the following
 * macros can be defined and used to compute pixel addresses during
 * rasterization (see pRow):
 *    PIXEL_TYPE          - the datatype of a pixel (GLubyte, GLushort, GLuint)
 *    BYTES_PER_ROW       - number of bytes per row in the color buffer
 *    PIXEL_ADDRESS(X,Y)  - returns the address of pixel at (X,Y) where
 *                          Y==0 at bottom of screen and increases upward.
 *
 * Similarly, for direct depth buffer access, this type is used for depth
 * buffer addressing:
 *    DEPTH_TYPE          - either GLushort or GLuint
 *
 * Optionally, one may provide one-time setup code per triangle:
 *    SETUP_CODE    - code which is to be executed once per triangle
 *    CLEANUP_CODE    - code to execute at end of triangle
 *
 * The following macro MUST be defined:
 *    RENDER_SPAN(span) - code to write a span of pixels.
 *
 * This code was designed for the origin to be in the lower-left corner.
 *
 * Inspired by triangle rasterizer code written by Allen Akin.  Thanks Allen!
 */


/*
 * This is a bit of a hack, but it's a centralized place to enable floating-
 * point color interpolation when GLchan is actually floating point.
 */



static void flat_rgba_triangle(GLcontext *ctx, const SWvertex *v0,
                                 const SWvertex *v1,
                                 const SWvertex *v2 )
{
   typedef struct {
        const SWvertex *v0, *v1;   /* Y(v0) < Y(v1) */
        GLfloat dx;    /* X(v1) - X(v0) */
        GLfloat dy;    /* Y(v1) - Y(v0) */
        GLfixed fdxdy; /* dx/dy in fixed-point */
        GLfixed fsx;   /* first sample point x coord */
        GLfixed fsy;
        GLfloat adjy;  /* adjust from v[0]->fy to fsy, scaled */
        GLint lines;   /* number of lines to be sampled on this edge */
        GLfixed fx0;   /* fixed pt X of lower endpoint */
   } EdgeT;

   const GLint depthBits = ctx->Visual.depthBits;
   const GLfloat maxDepth = ctx->DepthMaxF;
   EdgeT eMaj, eTop, eBot;
   GLfloat oneOverArea;
   const SWvertex *vMin, *vMid, *vMax;  /* Y(vMin)<=Y(vMid)<=Y(vMax) */
   float bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;
   const GLint snapMask = ~((0x00000800 / (1 << 4)) - 1); /* for x/y coord snapping */
   GLfixed vMin_fx, vMin_fy, vMid_fx, vMid_fy, vMax_fx, vMax_fy;

   struct sw_span span;

   (span).primitive = (0x0009);
   (span).interpMask = (0);
   (span).arrayMask = (0);
   (span).start = 0;
   (span).end = (0);
   (span).facing = 0;
   (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays;


   /*
   printf("%s()\n", __FUNCTION__);
   printf("  %g, %g, %g\n", v0->win[0], v0->win[1], v0->win[2]);
   printf("  %g, %g, %g\n", v1->win[0], v1->win[1], v1->win[2]);
   printf("  %g, %g, %g\n", v2->win[0], v2->win[1], v2->win[2]);
    */

   /* Compute fixed point x,y coords w/ half-pixel offsets and snapping.
    * And find the order of the 3 vertices along the Y axis.
    */
   {
      const GLfixed fy0 = FloatToFixed(v0->win[1] - 0.5F) & snapMask;
      const GLfixed fy1 = FloatToFixed(v1->win[1] - 0.5F) & snapMask;
      const GLfixed fy2 = FloatToFixed(v2->win[1] - 0.5F) & snapMask;

      if (fy0 <= fy1) {
         if (fy1 <= fy2) {
            /* y0 <= y1 <= y2 */
            vMin = v0;   vMid = v1;   vMax = v2;
            vMin_fy = fy0;  vMid_fy = fy1;  vMax_fy = fy2;
         }
         else if (fy2 <= fy0) {
            /* y2 <= y0 <= y1 */
            vMin = v2;   vMid = v0;   vMax = v1;
            vMin_fy = fy2;  vMid_fy = fy0;  vMax_fy = fy1;
         }
         else {
            /* y0 <= y2 <= y1 */
            vMin = v0;   vMid = v2;   vMax = v1;
            vMin_fy = fy0;  vMid_fy = fy2;  vMax_fy = fy1;
            bf = -bf;
         }
      }
      else {
         if (fy0 <= fy2) {
            /* y1 <= y0 <= y2 */
            vMin = v1;   vMid = v0;   vMax = v2;
            vMin_fy = fy1;  vMid_fy = fy0;  vMax_fy = fy2;
            bf = -bf;
         }
         else if (fy2 <= fy1) {
            /* y2 <= y1 <= y0 */
            vMin = v2;   vMid = v1;   vMax = v0;
            vMin_fy = fy2;  vMid_fy = fy1;  vMax_fy = fy0;
            bf = -bf;
         }
         else {
            /* y1 <= y2 <= y0 */
            vMin = v1;   vMid = v2;   vMax = v0;
            vMin_fy = fy1;  vMid_fy = fy2;  vMax_fy = fy0;
         }
      }

      /* fixed point X coords */
      vMin_fx = FloatToFixed(vMin->win[0] + 0.5F) & snapMask;
      vMid_fx = FloatToFixed(vMid->win[0] + 0.5F) & snapMask;
      vMax_fx = FloatToFixed(vMax->win[0] + 0.5F) & snapMask;
   }

   /* vertex/edge relationship */
   eMaj.v0 = vMin;   eMaj.v1 = vMax;   /*TODO: .v1's not needed */
   eTop.v0 = vMid;   eTop.v1 = vMax;
   eBot.v0 = vMin;   eBot.v1 = vMid;

   /* compute deltas for each edge:  vertex[upper] - vertex[lower] */
   eMaj.dx = FixedToFloat(vMax_fx - vMin_fx);
   eMaj.dy = FixedToFloat(vMax_fy - vMin_fy);
   eTop.dx = FixedToFloat(vMax_fx - vMid_fx);
   eTop.dy = FixedToFloat(vMax_fy - vMid_fy);
   eBot.dx = FixedToFloat(vMid_fx - vMin_fx);
   eBot.dy = FixedToFloat(vMid_fy - vMin_fy);

   /* compute area, oneOverArea and perform backface culling */
   {
      const GLfloat area = eMaj.dx * eBot.dy - eBot.dx * eMaj.dy;

      /* Do backface culling */
      if (area * bf < 0.0)
         return;

      if (IS_INF_OR_NAN(area) || area == 0.0F)
         return;

      oneOverArea = 1.0F / area;
   }

   ctx->OcclusionResult = GL_TRUE;
   span.facing = ctx->_Facing; /* for 2-sided stencil test */

   /* Edge setup.  For a triangle strip these could be reused... */
   {
      eMaj.fsy =  FixedCeil(vMin_fy);
      eMaj.lines =  FixedToInt(FixedCeil(vMax_fy - eMaj.fsy));
      if (eMaj.lines > 0) {
         GLfloat dxdy = eMaj.dx / eMaj.dy;
         eMaj.fdxdy = SignedFloatToFixed(dxdy);
         eMaj.adjy = (GLfloat) (eMaj.fsy - vMin_fy);  /* SCALED! */
         eMaj.fx0 = vMin_fx;
         eMaj.fsx = eMaj.fx0 + (GLfixed) (eMaj.adjy * dxdy);
      }
      else {
         return;  /*CULLED*/
      }

      eTop.fsy = FixedCeil(vMid_fy);
      eTop.lines = FixedToInt(FixedCeil(vMax_fy - eTop.fsy));
      if (eTop.lines > 0) {
         GLfloat dxdy = eTop.dx / eTop.dy;
         eTop.fdxdy = SignedFloatToFixed(dxdy);
         eTop.adjy = (GLfloat) (eTop.fsy - vMid_fy); /* SCALED! */
         eTop.fx0 = vMid_fx;
         eTop.fsx = eTop.fx0 + (GLfixed) (eTop.adjy * dxdy);
      }

      eBot.fsy = FixedCeil(vMin_fy);
      eBot.lines = FixedToInt(FixedCeil(vMid_fy - eBot.fsy));
      if (eBot.lines > 0) {
         GLfloat dxdy = eBot.dx / eBot.dy;
         eBot.fdxdy = SignedFloatToFixed(dxdy);
         eBot.adjy = (GLfloat) (eBot.fsy - vMin_fy);  /* SCALED! */
         eBot.fx0 = vMin_fx;
         eBot.fsx = eBot.fx0 + (GLfixed) (eBot.adjy * dxdy);
      }
   }

   /*
    * Conceptually, we view a triangle as two subtriangles
    * separated by a perfectly horizontal line.  The edge that is
    * intersected by this line is one with maximal absolute dy; we
    * call it a ``major'' edge.  The other two edges are the
    * ``top'' edge (for the upper subtriangle) and the ``bottom''
    * edge (for the lower subtriangle).  If either of these two
    * edges is horizontal or very close to horizontal, the
    * corresponding subtriangle might cover zero sample points;
    * we take care to handle such cases, for performance as well
    * as correctness.
    *
    * By stepping rasterization parameters along the major edge,
    * we can avoid recomputing them at the discontinuity where
    * the top and bottom edges meet.  However, this forces us to
    * be able to scan both left-to-right and right-to-left.
    * Also, we must determine whether the major edge is at the
    * left or right side of the triangle.  We do this by
    * computing the magnitude of the cross-product of the major
    * and top edges.  Since this magnitude depends on the sine of
    * the angle between the two edges, its sign tells us whether
    * we turn to the left or to the right when travelling along
    * the major edge to the top edge, and from this we infer
    * whether the major edge is on the left or the right.
    *
    * Serendipitously, this cross-product magnitude is also a
    * value we need to compute the iteration parameter
    * derivatives for the triangle, and it can be used to perform
    * backface culling because its sign tells us whether the
    * triangle is clockwise or counterclockwise.  In this code we
    * refer to it as ``area'' because it's also proportional to
    * the pixel area of the triangle.
    */

   {
      GLint scan_from_left_to_right;  /* true if scanning left-to-right */
      GLfloat dzdx, dzdy;
      GLfloat dfogdy;

      /*
       * Execute user-supplied setup code
       */
       span.interpMask |= 0x001;
       span.red = ((v2->color[0]) << 11);
       span.green = ((v2->color[1]) << 11);
       span.blue = ((v2->color[2]) << 11);
       span.alpha = ((v2->color[3]) << 11);
       span.redStep = 0; span.greenStep = 0; span.blueStep = 0; span.alphaStep = 0;

      scan_from_left_to_right = (oneOverArea < 0.0F);


      /* compute d?/dx and d?/dy derivatives */
      span.interpMask |=  SPAN_Z;
      {
         GLfloat eMaj_dz, eBot_dz;
         eMaj_dz = vMax->win[2] - vMin->win[2];
         eBot_dz = vMid->win[2] - vMin->win[2];
         dzdx = oneOverArea * (eMaj_dz * eBot.dy - eMaj.dy * eBot_dz);
         if (dzdx > maxDepth || dzdx < -maxDepth) {
            /* probably a sliver triangle */
            dzdx = 0.0;
            dzdy = 0.0;
         }
         else {
            dzdy = oneOverArea * (eMaj.dx * eBot_dz - eMaj_dz * eBot.dx);
         }
         if (depthBits <= 16)
            span.zStep = SignedFloatToFixed(dzdx);
         else
            span.zStep = (GLint) dzdx;
      }
      span.interpMask |=  SPAN_FOG;
      {
         const GLfloat eMaj_dfog = vMax->fog - vMin->fog;
         const GLfloat eBot_dfog = vMid->fog - vMin->fog;
         span.fogStep = oneOverArea * (eMaj_dfog * eBot.dy - eMaj.dy * eBot_dfog);
         dfogdy = oneOverArea * (eMaj.dx * eBot_dfog - eMaj_dfog * eBot.dx);
      }

      /*
       * We always sample at pixel centers.  However, we avoid
       * explicit half-pixel offsets in this code by incorporating
       * the proper offset in each of x and y during the
       * transformation to window coordinates.
       *
       * We also apply the usual rasterization rules to prevent
       * cracks and overlaps.  A pixel is considered inside a
       * subtriangle if it meets all of four conditions: it is on or
       * to the right of the left edge, strictly to the left of the
       * right edge, on or below the top edge, and strictly above
       * the bottom edge.  (Some edges may be degenerate.)
       *
       * The following discussion assumes left-to-right scanning
       * (that is, the major edge is on the left); the right-to-left
       * case is a straightforward variation.
       *
       * We start by finding the half-integral y coordinate that is
       * at or below the top of the triangle.  This gives us the
       * first scan line that could possibly contain pixels that are
       * inside the triangle.
       *
       * Next we creep down the major edge until we reach that y,
       * and compute the corresponding x coordinate on the edge.
       * Then we find the half-integral x that lies on or just
       * inside the edge.  This is the first pixel that might lie in
       * the interior of the triangle.  (We won't know for sure
       * until we check the other edges.)
       *
       * As we rasterize the triangle, we'll step down the major
       * edge.  For each step in y, we'll move an integer number
       * of steps in x.  There are two possible x step sizes, which
       * we'll call the ``inner'' step (guaranteed to land on the
       * edge or inside it) and the ``outer'' step (guaranteed to
       * land on the edge or outside it).  The inner and outer steps
       * differ by one.  During rasterization we maintain an error
       * term that indicates our distance from the true edge, and
       * select either the inner step or the outer step, whichever
       * gets us to the first pixel that falls inside the triangle.
       *
       * All parameters (z, red, etc.) as well as the buffer
       * addresses for color and z have inner and outer step values,
       * so that we can increment them appropriately.  This method
       * eliminates the need to adjust parameters by creeping a
       * sub-pixel amount into the triangle at each scanline.
       */

      {
         int subTriangle;
         GLfixed fx;
         GLfixed fxLeftEdge = 0, fxRightEdge = 0;
         GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
         GLfixed fdxOuter;
         int idxOuter;
         float dxOuter;
         GLfixed fError = 0, fdError = 0;
         float adjx, adjy;
         GLfixed fy;
         GLushort *zRow = 0;
         int dZRowOuter = 0, dZRowInner;  /* offset in bytes */
         GLfixed fz = 0, fdzOuter = 0, fdzInner;
         GLfloat fogLeft = 0, dfogOuter = 0, dfogInner;

         for (subTriangle=0; subTriangle<=1; subTriangle++) {
            EdgeT *eLeft, *eRight;
            int setupLeft, setupRight;
            int lines;

            if (subTriangle==0) {
               /* bottom half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eBot;
                  lines = eRight->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
               else {
                  eLeft = &eBot;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
            }
            else {
               /* top half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eTop;
                  lines = eRight->lines;
                  setupLeft = 0;
                  setupRight = 1;
               }
               else {
                  eLeft = &eTop;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 0;
               }
               if (lines == 0)
                  return;
            }

            if (setupLeft && eLeft->lines > 0) {
               const SWvertex *vLower;
               GLfixed fsx = eLeft->fsx;
               fx = (((fsx) + 0x00000800 - 1) & (~0x000007FF));
               fError = fx - fsx - 0x00000800;
               fxLeftEdge = fsx - 1;
               fdxLeftEdge = eLeft->fdxdy;
               fdxOuter = ((fdxLeftEdge - 1) & (~0x000007FF));
               fdError = fdxOuter - fdxLeftEdge + 0x00000800;
               idxOuter = ((fdxOuter) >> 11);
               dxOuter = (float) idxOuter;
               (void) dxOuter;

               fy = eLeft->fsy;
               span.y = ((fy) >> 11);

               adjx = (float)(fx - eLeft->fx0);  /* SCALED! */
               adjy = eLeft->adjy;              /* SCALED! */
               vLower = eLeft->v0;

               /*
                * Now we need the set of parameter (z, color, etc.) values at
                * the point (fx, fy).  This gives us properly-sampled parameter
                * values that we can step from pixel to pixel.  Furthermore,
                * although we might have intermediate results that overflow
                * the normal parameter range when we step temporarily outside
                * the triangle, we shouldn't overflow or underflow for any
                * pixel that's actually inside the triangle.
                */

               {
                  GLfloat z0 = vLower->win[2];
                  if (depthBits <= 16) {
                     /* interpolate fixed-pt values */
                     GLfloat tmp = (z0 * FIXED_SCALE  +
                                    dzdx * adjx + dzdy * adjy) +  FIXED_HALF;
                     if (tmp < MAX_GLUINT / 2)
                        fz = (GLfixed) tmp;
                     else
                        fz = MAX_GLUINT / 2;
                     fdzOuter = SignedFloatToFixed(dzdy + dxOuter * dzdx);
                  }
                  else {
                     /* interpolate depth values exactly */
                     fz = (GLint) (z0 + dzdx * FixedToFloat(adjx)
                                   + dzdy * FixedToFloat(adjy));
                     fdzOuter = (GLint) (dzdy + dxOuter * dzdx);
                  }

                  zRow = (DEFAULT_SOFTWARE_DEPTH_TYPE *)
                    _mesa_zbuffer_address(ctx, FixedToInt(fxLeftEdge), span.y);
                  dZRowOuter = (ctx->DrawBuffer->Width + idxOuter) * sizeof(DEFAULT_SOFTWARE_DEPTH_TYPE);
               }
               fogLeft = vLower->fog + (span.fogStep * adjx + dfogdy * adjy)
                                       * (1.0F/2048.0f);
               dfogOuter = dfogdy + dxOuter * span.fogStep;

            } /*if setupLeft*/


            if (setupRight && eRight->lines>0) {
               fxRightEdge = eRight->fsx - 1;
               fdxRightEdge = eRight->fdxdy;
            }

            if (lines==0) {
               continue;
            }


            /* Rasterize setup */
            dZRowInner = dZRowOuter + sizeof(GLushort);
            fdzInner = fdzOuter + span.zStep;
            dfogInner = dfogOuter + span.fogStep;

            while (lines > 0) {
               /* initialize the span interpolants to the leftmost value */
               /* ff = fixed-pt fragment */
               const GLint right = ((fxRightEdge) >> 11);

               span.x = ((fxLeftEdge) >> 11);

               if (right <= span.x)
                  span.end = 0;
               else
                  span.end = right - span.x;

               span.z = fz;
               span.fog = fogLeft;




               /* This is where we actually generate fragments */
               if (span.end > 0) {
                  _mesa_write_rgba_span(ctx, &span);;
               }

               /*
                * Advance to the next scan line.  Compute the
                * new edge coordinates, and adjust the
                * pixel-center x coordinate so that it stays
                * on or inside the major edge.
                */
               (span.y)++;
               lines--;

               fxLeftEdge += fdxLeftEdge;
               fxRightEdge += fdxRightEdge;


               fError += fdError;
               if (fError >= 0) {
                  fError -= 0x00000800;
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowOuter);
                  fz += fdzOuter;
                  fogLeft += dfogOuter;
               }
               else {
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowInner);
                  fz += fdzInner;
                  fogLeft += dfogInner;
               }
            } /*while lines>0*/

         } /* for subTriangle */

      }
   }
}

/*
 * Render a smooth-shaded RGBA triangle.
 */

/*
 * Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom triangle rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated across the triangle:
 *    INTERP_Z        - if defined, interpolate Z values
 *    INTERP_FOG      - if defined, interpolate fog values
 *    INTERP_RGB      - if defined, interpolate RGB values
 *    INTERP_ALPHA    - if defined, interpolate Alpha values (req's INTERP_RGB)
 *    INTERP_SPEC     - if defined, interpolate specular RGB values
 *    INTERP_INDEX    - if defined, interpolate color index values
 *    INTERP_INT_TEX  - if defined, interpolate integer ST texcoords
 *                         (fast, simple 2-D texture mapping)
 *    INTERP_TEX      - if defined, interpolate set 0 float STRQ texcoords
 *                         NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_MULTITEX - if defined, interpolate N units of STRQ texcoords
 *    INTERP_FLOAT_RGBA - if defined, interpolate RGBA with floating point
 *    INTERP_FLOAT_SPEC - if defined, interpolate specular with floating point
 *
 * When one can directly address pixels in the color buffer the following
 * macros can be defined and used to compute pixel addresses during
 * rasterization (see pRow):
 *    PIXEL_TYPE          - the datatype of a pixel (GLubyte, GLushort, GLuint)
 *    BYTES_PER_ROW       - number of bytes per row in the color buffer
 *    PIXEL_ADDRESS(X,Y)  - returns the address of pixel at (X,Y) where
 *                          Y==0 at bottom of screen and increases upward.
 *
 * Similarly, for direct depth buffer access, this type is used for depth
 * buffer addressing:
 *    DEPTH_TYPE          - either GLushort or GLuint
 *
 * Optionally, one may provide one-time setup code per triangle:
 *    SETUP_CODE    - code which is to be executed once per triangle
 *    CLEANUP_CODE    - code to execute at end of triangle
 *
 * The following macro MUST be defined:
 *    RENDER_SPAN(span) - code to write a span of pixels.
 *
 * This code was designed for the origin to be in the lower-left corner.
 *
 * Inspired by triangle rasterizer code written by Allen Akin.  Thanks Allen!
 */


/*
 * This is a bit of a hack, but it's a centralized place to enable floating-
 * point color interpolation when GLchan is actually floating point.
 */



static void smooth_rgba_triangle(GLcontext *ctx, const SWvertex *v0,
                                 const SWvertex *v1,
                                 const SWvertex *v2 )
{
   typedef struct {
        const SWvertex *v0, *v1;   /* Y(v0) < Y(v1) */
        GLfloat dx;    /* X(v1) - X(v0) */
        GLfloat dy;    /* Y(v1) - Y(v0) */
        GLfixed fdxdy; /* dx/dy in fixed-point */
        GLfixed fsx;   /* first sample point x coord */
        GLfixed fsy;
        GLfloat adjy;  /* adjust from v[0]->fy to fsy, scaled */
        GLint lines;   /* number of lines to be sampled on this edge */
        GLfixed fx0;   /* fixed pt X of lower endpoint */
   } EdgeT;

   const GLint depthBits = ctx->Visual.depthBits;
   const GLfloat maxDepth = ctx->DepthMaxF;
   EdgeT eMaj, eTop, eBot;
   GLfloat oneOverArea;
   const SWvertex *vMin, *vMid, *vMax;  /* Y(vMin)<=Y(vMid)<=Y(vMax) */
   float bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;
   const GLint snapMask = ~((FIXED_ONE / (1 << SUB_PIXEL_BITS)) - 1); /* for x/y coord snapping */

   GLfixed vMin_fx, vMin_fy, vMid_fx, vMid_fy, vMax_fx, vMax_fy;

   struct sw_span span;

    (span).primitive = (0x0009);
    (span).interpMask = (0);
    (span).arrayMask = (0);
    (span).start = 0;
    (span).end = (0);
    (span).facing = 0;
    (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays;


   /*
   printf("%s()\n", __FUNCTION__);
   printf("  %g, %g, %g\n", v0->win[0], v0->win[1], v0->win[2]);
   printf("  %g, %g, %g\n", v1->win[0], v1->win[1], v1->win[2]);
   printf("  %g, %g, %g\n", v2->win[0], v2->win[1], v2->win[2]);
    */

   /* Compute fixed point x,y coords w/ half-pixel offsets and snapping.
    * And find the order of the 3 vertices along the Y axis.
    */
   {
      const GLfixed fy0 = FloatToFixed(v0->win[1] - 0.5F) & snapMask;
      const GLfixed fy1 = FloatToFixed(v1->win[1] - 0.5F) & snapMask;
      const GLfixed fy2 = FloatToFixed(v2->win[1] - 0.5F) & snapMask;

      if (fy0 <= fy1) {
         if (fy1 <= fy2) {
            /* y0 <= y1 <= y2 */
            vMin = v0;   vMid = v1;   vMax = v2;
            vMin_fy = fy0;  vMid_fy = fy1;  vMax_fy = fy2;
         }
         else if (fy2 <= fy0) {
            /* y2 <= y0 <= y1 */
            vMin = v2;   vMid = v0;   vMax = v1;
            vMin_fy = fy2;  vMid_fy = fy0;  vMax_fy = fy1;
         }
         else {
            /* y0 <= y2 <= y1 */
            vMin = v0;   vMid = v2;   vMax = v1;
            vMin_fy = fy0;  vMid_fy = fy2;  vMax_fy = fy1;
            bf = -bf;
         }
      }
      else {
         if (fy0 <= fy2) {
            /* y1 <= y0 <= y2 */
            vMin = v1;   vMid = v0;   vMax = v2;
            vMin_fy = fy1;  vMid_fy = fy0;  vMax_fy = fy2;
            bf = -bf;
         }
         else if (fy2 <= fy1) {
            /* y2 <= y1 <= y0 */
            vMin = v2;   vMid = v1;   vMax = v0;
            vMin_fy = fy2;  vMid_fy = fy1;  vMax_fy = fy0;
            bf = -bf;
         }
         else {
            /* y1 <= y2 <= y0 */
            vMin = v1;   vMid = v2;   vMax = v0;
            vMin_fy = fy1;  vMid_fy = fy2;  vMax_fy = fy0;
         }
      }

      /* fixed point X coords */
      vMin_fx = FloatToFixed(vMin->win[0] + 0.5F) & snapMask;
      vMid_fx = FloatToFixed(vMid->win[0] + 0.5F) & snapMask;
      vMax_fx = FloatToFixed(vMax->win[0] + 0.5F) & snapMask;
   }


   /* vertex/edge relationship */
   eMaj.v0 = vMin;   eMaj.v1 = vMax;   /*TODO: .v1's not needed */
   eTop.v0 = vMid;   eTop.v1 = vMax;
   eBot.v0 = vMin;   eBot.v1 = vMid;

   /* compute deltas for each edge:  vertex[upper] - vertex[lower] */
   eMaj.dx = FixedToFloat(vMax_fx - vMin_fx);
   eMaj.dy = FixedToFloat(vMax_fy - vMin_fy);
   eTop.dx = FixedToFloat(vMax_fx - vMid_fx);
   eTop.dy = FixedToFloat(vMax_fy - vMid_fy);
   eBot.dx = FixedToFloat(vMid_fx - vMin_fx);
   eBot.dy = FixedToFloat(vMid_fy - vMin_fy);

   /* compute area, oneOverArea and perform backface culling */
   {
      const GLfloat area = eMaj.dx * eBot.dy - eBot.dx * eMaj.dy;

      /* Do backface culling */
      if (area * bf < 0.0)
         return;

      if (IS_INF_OR_NAN(area) || area == 0.0F)
         return;

      oneOverArea = 1.0F / area;
   }

   ctx->OcclusionResult = 0x1;
   span.facing = ctx->_Facing; /* for 2-sided stencil test */

   /* Edge setup.  For a triangle strip these could be reused... */
   {
      eMaj.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eMaj.lines = (((((vMax_fy - eMaj.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eMaj.lines > 0) {
         GLfloat dxdy = eMaj.dx / eMaj.dy;
         eMaj.fdxdy = SignedFloatToFixed(dxdy);
         eMaj.adjy = (GLfloat) (eMaj.fsy - vMin_fy);  /* SCALED! */
         eMaj.fx0 = vMin_fx;
         eMaj.fsx = eMaj.fx0 + (GLfixed) (eMaj.adjy * dxdy);
      }
      else {
         return;  /*CULLED*/
      }

      eTop.fsy = (((vMid_fy) + 0x00000800 - 1) & (~0x000007FF));
      eTop.lines = (((((vMax_fy - eTop.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eTop.lines > 0) {
         GLfloat dxdy = eTop.dx / eTop.dy;
         eTop.fdxdy = SignedFloatToFixed(dxdy);
         eTop.adjy = (GLfloat) (eTop.fsy - vMid_fy); /* SCALED! */
         eTop.fx0 = vMid_fx;
         eTop.fsx = eTop.fx0 + (GLfixed) (eTop.adjy * dxdy);
      }

      eBot.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eBot.lines = (((((vMid_fy - eBot.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eBot.lines > 0) {
         GLfloat dxdy = eBot.dx / eBot.dy;
         eBot.fdxdy = SignedFloatToFixed(dxdy);
         eBot.adjy = (GLfloat) (eBot.fsy - vMin_fy);  /* SCALED! */
         eBot.fx0 = vMin_fx;
         eBot.fsx = eBot.fx0 + (GLfixed) (eBot.adjy * dxdy);
      }
   }

   /*
    * Conceptually, we view a triangle as two subtriangles
    * separated by a perfectly horizontal line.  The edge that is
    * intersected by this line is one with maximal absolute dy; we
    * call it a ``major'' edge.  The other two edges are the
    * ``top'' edge (for the upper subtriangle) and the ``bottom''
    * edge (for the lower subtriangle).  If either of these two
    * edges is horizontal or very close to horizontal, the
    * corresponding subtriangle might cover zero sample points;
    * we take care to handle such cases, for performance as well
    * as correctness.
    *
    * By stepping rasterization parameters along the major edge,
    * we can avoid recomputing them at the discontinuity where
    * the top and bottom edges meet.  However, this forces us to
    * be able to scan both left-to-right and right-to-left.
    * Also, we must determine whether the major edge is at the
    * left or right side of the triangle.  We do this by
    * computing the magnitude of the cross-product of the major
    * and top edges.  Since this magnitude depends on the sine of
    * the angle between the two edges, its sign tells us whether
    * we turn to the left or to the right when travelling along
    * the major edge to the top edge, and from this we infer
    * whether the major edge is on the left or the right.
    *
    * Serendipitously, this cross-product magnitude is also a
    * value we need to compute the iteration parameter
    * derivatives for the triangle, and it can be used to perform
    * backface culling because its sign tells us whether the
    * triangle is clockwise or counterclockwise.  In this code we
    * refer to it as ``area'' because it's also proportional to
    * the pixel area of the triangle.
    */

   {
      GLint scan_from_left_to_right;  /* true if scanning left-to-right */
      GLfloat dzdx, dzdy;
      GLfloat dfogdy;
      GLfloat drdx, drdy;
      GLfloat dgdx, dgdy;
      GLfloat dbdx, dbdy;
      GLfloat dadx, dady;

      /*
       * Execute user-supplied setup code
       */
/* ..... */
      scan_from_left_to_right = (oneOverArea < 0.0F);


      /* compute d?/dx and d?/dy derivatives */
      span.interpMask |= SPAN_Z;
      {
         GLfloat eMaj_dz, eBot_dz;
         eMaj_dz = vMax->win[2] - vMin->win[2];
         eBot_dz = vMid->win[2] - vMin->win[2];
         dzdx = oneOverArea * (eMaj_dz * eBot.dy - eMaj.dy * eBot_dz);
         if (dzdx > maxDepth || dzdx < -maxDepth) {
            /* probably a sliver triangle */
            dzdx = 0.0;
            dzdy = 0.0;
         }
         else {
            dzdy = oneOverArea * (eMaj.dx * eBot_dz - eMaj_dz * eBot.dx);
         }
         if (depthBits <= 16)
            span.zStep = SignedFloatToFixed(dzdx);
         else
            span.zStep = (GLint) dzdx;
      }
      span.interpMask |= SPAN_FOG;
      {
         const GLfloat eMaj_dfog = vMax->fog - vMin->fog;
         const GLfloat eBot_dfog = vMid->fog - vMin->fog;
         span.fogStep = oneOverArea * (eMaj_dfog * eBot.dy - eMaj.dy * eBot_dfog);
         dfogdy = oneOverArea * (eMaj.dx * eBot_dfog - eMaj_dfog * eBot.dx);
      }
      span.interpMask |= SPAN_RGBA;
      if (ctx->Light.ShadeModel == GL_SMOOTH) {
         GLfloat eMaj_dr, eBot_dr;
         GLfloat eMaj_dg, eBot_dg;
         GLfloat eMaj_db, eBot_db;
         GLfloat eMaj_da, eBot_da;
         eMaj_dr = (GLfloat) ((GLint) vMax->color[RCOMP] -
                             (GLint) vMin->color[RCOMP]);
         eBot_dr = (GLfloat) ((GLint) vMid->color[RCOMP] -
                             (GLint) vMin->color[RCOMP]);
         drdx = oneOverArea * (eMaj_dr * eBot.dy - eMaj.dy * eBot_dr);
         span.redStep = SignedFloatToFixed(drdx);
         drdy = oneOverArea * (eMaj.dx * eBot_dr - eMaj_dr * eBot.dx);
         eMaj_dg = (GLfloat) ((GLint) vMax->color[GCOMP] -
                             (GLint) vMin->color[GCOMP]);
         eBot_dg = (GLfloat) ((GLint) vMid->color[GCOMP] -
                             (GLint) vMin->color[GCOMP]);
         dgdx = oneOverArea * (eMaj_dg * eBot.dy - eMaj.dy * eBot_dg);
         span.greenStep = SignedFloatToFixed(dgdx);
         dgdy = oneOverArea * (eMaj.dx * eBot_dg - eMaj_dg * eBot.dx);
         eMaj_db = (GLfloat) ((GLint) vMax->color[BCOMP] -
                             (GLint) vMin->color[BCOMP]);
         eBot_db = (GLfloat) ((GLint) vMid->color[BCOMP] -
                             (GLint) vMin->color[BCOMP]);
         dbdx = oneOverArea * (eMaj_db * eBot.dy - eMaj.dy * eBot_db);
         span.blueStep = SignedFloatToFixed(dbdx);
         dbdy = oneOverArea * (eMaj.dx * eBot_db - eMaj_db * eBot.dx);
         eMaj_da = (GLfloat) ((GLint) vMax->color[ACOMP] -
                             (GLint) vMin->color[ACOMP]);
         eBot_da = (GLfloat) ((GLint) vMid->color[ACOMP] -
                             (GLint) vMin->color[ACOMP]);
         dadx = oneOverArea * (eMaj_da * eBot.dy - eMaj.dy * eBot_da);
         span.alphaStep = SignedFloatToFixed(dadx);
         dady = oneOverArea * (eMaj.dx * eBot_da - eMaj_da * eBot.dx);
      }
      else {
         span.interpMask |= SPAN_FLAT;
         drdx = drdy = 0.0F;
         dgdx = dgdy = 0.0F;
         dbdx = dbdy = 0.0F;
         span.redStep = 0;
         span.greenStep = 0;
         span.blueStep = 0;
         dadx = dady = 0.0F;
         span.alphaStep = 0;
      }

      /*
       * We always sample at pixel centers.  However, we avoid
       * explicit half-pixel offsets in this code by incorporating
       * the proper offset in each of x and y during the
       * transformation to window coordinates.
       *
       * We also apply the usual rasterization rules to prevent
       * cracks and overlaps.  A pixel is considered inside a
       * subtriangle if it meets all of four conditions: it is on or
       * to the right of the left edge, strictly to the left of the
       * right edge, on or below the top edge, and strictly above
       * the bottom edge.  (Some edges may be degenerate.)
       *
       * The following discussion assumes left-to-right scanning
       * (that is, the major edge is on the left); the right-to-left
       * case is a straightforward variation.
       *
       * We start by finding the half-integral y coordinate that is
       * at or below the top of the triangle.  This gives us the
       * first scan line that could possibly contain pixels that are
       * inside the triangle.
       *
       * Next we creep down the major edge until we reach that y,
       * and compute the corresponding x coordinate on the edge.
       * Then we find the half-integral x that lies on or just
       * inside the edge.  This is the first pixel that might lie in
       * the interior of the triangle.  (We won't know for sure
       * until we check the other edges.)
       *
       * As we rasterize the triangle, we'll step down the major
       * edge.  For each step in y, we'll move an integer number
       * of steps in x.  There are two possible x step sizes, which
       * we'll call the ``inner'' step (guaranteed to land on the
       * edge or inside it) and the ``outer'' step (guaranteed to
       * land on the edge or outside it).  The inner and outer steps
       * differ by one.  During rasterization we maintain an error
       * term that indicates our distance from the true edge, and
       * select either the inner step or the outer step, whichever
       * gets us to the first pixel that falls inside the triangle.
       *
       * All parameters (z, red, etc.) as well as the buffer
       * addresses for color and z have inner and outer step values,
       * so that we can increment them appropriately.  This method
       * eliminates the need to adjust parameters by creeping a
       * sub-pixel amount into the triangle at each scanline.
       */

      {
         int subTriangle;
         GLfixed fx;
         GLfixed fxLeftEdge = 0, fxRightEdge = 0;
         GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
         GLfixed fdxOuter;
         int idxOuter;
         float dxOuter;
         GLfixed fError = 0, fdError = 0;
         float adjx, adjy;
         GLfixed fy;
         GLushort *zRow = 0;
         int dZRowOuter = 0, dZRowInner;  /* offset in bytes */
         GLfixed fz = 0, fdzOuter = 0, fdzInner;
         GLfloat fogLeft = 0, dfogOuter = 0, dfogInner;
         GLfixed fr = 0, fdrOuter = 0, fdrInner;
         GLfixed fg = 0, fdgOuter = 0, fdgInner;
         GLfixed fb = 0, fdbOuter = 0, fdbInner;
         GLfixed fa = 0, fdaOuter = 0, fdaInner;

         for (subTriangle=0; subTriangle<=1; subTriangle++) {
            EdgeT *eLeft, *eRight;
            int setupLeft, setupRight;
            int lines;

            if (subTriangle==0) {
               /* bottom half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eBot;
                  lines = eRight->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
               else {
                  eLeft = &eBot;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
            }
            else {
               /* top half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eTop;
                  lines = eRight->lines;
                  setupLeft = 0;
                  setupRight = 1;
               }
               else {
                  eLeft = &eTop;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 0;
               }
               if (lines == 0)
                  return;
            }

            if (setupLeft && eLeft->lines > 0) {
               const SWvertex *vLower;
               GLfixed fsx = eLeft->fsx;
               fx = (((fsx) + 0x00000800 - 1) & (~0x000007FF));
               fError = fx - fsx - 0x00000800;
               fxLeftEdge = fsx - 1;
               fdxLeftEdge = eLeft->fdxdy;
               fdxOuter = ((fdxLeftEdge - 1) & (~0x000007FF));
               fdError = fdxOuter - fdxLeftEdge + 0x00000800;
               idxOuter = ((fdxOuter) >> 11);
               dxOuter = (float) idxOuter;
               (void) dxOuter;

               fy = eLeft->fsy;
               span.y = ((fy) >> 11);

               adjx = (float)(fx - eLeft->fx0);  /* SCALED! */
               adjy = eLeft->adjy;              /* SCALED! */
               vLower = eLeft->v0;

               /*
                * Now we need the set of parameter (z, color, etc.) values at
                * the point (fx, fy).  This gives us properly-sampled parameter
                * values that we can step from pixel to pixel.  Furthermore,
                * although we might have intermediate results that overflow
                * the normal parameter range when we step temporarily outside
                * the triangle, we shouldn't overflow or underflow for any
                * pixel that's actually inside the triangle.
                */

               {
                  GLfloat z0 = vLower->win[2];
                  if (depthBits <= 16) {
                     /* interpolate fixed-pt values */
                     GLfloat tmp = (z0 * 2048.0f +
                                    dzdx * adjx + dzdy * adjy) + 0x00000400;
                     if (tmp < MAX_GLUINT / 2)
                        fz = (GLfixed) tmp;
                     else
                        fz = MAX_GLUINT / 2;
                     fdzOuter = SignedFloatToFixed(dzdy + dxOuter * dzdx);
                  }
                  else {
                     /* interpolate depth values exactly */
                     fz = (GLint) (z0 + dzdx * FixedToFloat(adjx)
                                   + dzdy * FixedToFloat(adjy));
                     fdzOuter = (GLint) (dzdy + dxOuter * dzdx);
                  }
                  zRow = (GLushort *)
                    _mesa_zbuffer_address(ctx, ((fxLeftEdge) >> 11), span.y);
                  dZRowOuter = (ctx->DrawBuffer->Width + idxOuter) * sizeof(GLushort);
               }
               fogLeft = vLower->fog + (span.fogStep * adjx + dfogdy * adjy)
                                       * (1.0F/2048.0f);
               dfogOuter = dfogdy + dxOuter * span.fogStep;
               if (ctx->Light.ShadeModel == GL_SMOOTH) {
                  fr = (GLfixed) (ChanToFixed(vLower->color[RCOMP])
                                   + drdx * adjx + drdy * adjy) + FIXED_HALF;
                  fdrOuter = SignedFloatToFixed(drdy + dxOuter * drdx);
                  fg = (GLfixed) (ChanToFixed(vLower->color[GCOMP])
                                   + dgdx * adjx + dgdy * adjy) + FIXED_HALF;
                  fdgOuter = SignedFloatToFixed(dgdy + dxOuter * dgdx);
                  fb = (GLfixed) (ChanToFixed(vLower->color[BCOMP])
                                    + dbdx * adjx + dbdy * adjy) + FIXED_HALF;
                  fdbOuter = SignedFloatToFixed(dbdy + dxOuter * dbdx);
                  fa = (GLfixed) (ChanToFixed(vLower->color[ACOMP])
                                   + dadx * adjx + dady * adjy) + FIXED_HALF;
                  fdaOuter = SignedFloatToFixed(dady + dxOuter * dadx);
               }
               else {
                  fr = ChanToFixed(v2->color[RCOMP]);
                  fg = ChanToFixed(v2->color[GCOMP]);
                  fdrOuter = fdgOuter = fdbOuter = 0;
                  fa =  ChanToFixed(v2->color[ACOMP]);
                  fdaOuter = 0;
               }

            } /*if setupLeft*/


            if (setupRight && eRight->lines>0) {
               fxRightEdge = eRight->fsx - 1;
               fdxRightEdge = eRight->fdxdy;
            }

            if (lines==0) {
               continue;
            }


            /* Rasterize setup */
            dZRowInner = dZRowOuter + sizeof(GLushort);
            fdzInner = fdzOuter + span.zStep;
            dfogInner = dfogOuter + span.fogStep;
            fdrInner = fdrOuter + span.redStep;
            fdgInner = fdgOuter + span.greenStep;
            fdbInner = fdbOuter + span.blueStep;
            fdaInner = fdaOuter + span.alphaStep;

            while (lines > 0) {
               /* initialize the span interpolants to the leftmost value */
               /* ff = fixed-pt fragment */
               const GLint right = ((fxRightEdge) >> 11);

               span.x = ((fxLeftEdge) >> 11);

               if (right <= span.x)
                  span.end = 0;
               else
                  span.end = right - span.x;

               span.z = fz;
               span.fog = fogLeft;
               span.red = fr;
               span.green = fg;
               span.blue = fb;
               span.alpha = fa;


               {
                  /* need this to accomodate round-off errors */
                  const GLint len = right - span.x - 1;
                  GLfixed ffrend = span.red + len * span.redStep;
                  GLfixed ffgend = span.green + len * span.greenStep;
                  GLfixed ffbend = span.blue + len * span.blueStep;
                  if (ffrend < 0) {
                     span.red -= ffrend;
                     if (span.red < 0)
                        span.red = 0;
                  }
                  if (ffgend < 0) {
                     span.green -= ffgend;
                     if (span.green < 0)
                        span.green = 0;
                  }
                  if (ffbend < 0) {
                     span.blue -= ffbend;
                     if (span.blue < 0)
                        span.blue = 0;
                  }
               }
               {
                  const GLint len = right - span.x - 1;
                  GLfixed ffaend = span.alpha + len * span.alphaStep;
                  if (ffaend < 0) {
                     span.alpha -= ffaend;
                     if (span.alpha < 0)
                        span.alpha = 0;
                  }
               }

               /* This is where we actually generate fragments */
               if (span.end > 0) {
                  _mesa_write_rgba_span(ctx, &span);
               }

               /*
                * Advance to the next scan line.  Compute the
                * new edge coordinates, and adjust the
                * pixel-center x coordinate so that it stays
                * on or inside the major edge.
                */
               (span.y)++;
               lines--;

               fxLeftEdge += fdxLeftEdge;
               fxRightEdge += fdxRightEdge;


               fError += fdError;
               if (fError >= 0) {
                  fError -= 0x00000800;
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowOuter);
                  fz += fdzOuter;
                  fogLeft += dfogOuter;
                  fr += fdrOuter;
                  fg += fdgOuter;
                  fb += fdbOuter;
                  fa += fdaOuter;
               }
               else {
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowInner);
                  fz += fdzInner;
                  fogLeft += dfogInner;
                  fr += fdrInner;
                  fg += fdgInner;
                  fb += fdbInner;
                  fa += fdaInner;
               }
            } /*while lines>0*/

         } /* for subTriangle */

      }
   }
}

/*
 * Render an RGB, GL_DECAL, textured triangle.
 * Interpolate S,T only w/out mipmapping or perspective correction.
 *
 * No fog.
 */

/*
 * Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom triangle rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated across the triangle:
 *    INTERP_Z        - if defined, interpolate Z values
 *    INTERP_FOG      - if defined, interpolate fog values
 *    INTERP_RGB      - if defined, interpolate RGB values
 *    INTERP_ALPHA    - if defined, interpolate Alpha values (req's INTERP_RGB)
 *    INTERP_SPEC     - if defined, interpolate specular RGB values
 *    INTERP_INDEX    - if defined, interpolate color index values
 *    INTERP_INT_TEX  - if defined, interpolate integer ST texcoords
 *                         (fast, simple 2-D texture mapping)
 *    INTERP_TEX      - if defined, interpolate set 0 float STRQ texcoords
 *                         NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_MULTITEX - if defined, interpolate N units of STRQ texcoords
 *    INTERP_FLOAT_RGBA - if defined, interpolate RGBA with floating point
 *    INTERP_FLOAT_SPEC - if defined, interpolate specular with floating point
 *
 * When one can directly address pixels in the color buffer the following
 * macros can be defined and used to compute pixel addresses during
 * rasterization (see pRow):
 *    PIXEL_TYPE          - the datatype of a pixel (GLubyte, GLushort, GLuint)
 *    BYTES_PER_ROW       - number of bytes per row in the color buffer
 *    PIXEL_ADDRESS(X,Y)  - returns the address of pixel at (X,Y) where
 *                          Y==0 at bottom of screen and increases upward.
 *
 * Similarly, for direct depth buffer access, this type is used for depth
 * buffer addressing:
 *    DEPTH_TYPE          - either GLushort or GLuint
 *
 * Optionally, one may provide one-time setup code per triangle:
 *    SETUP_CODE    - code which is to be executed once per triangle
 *    CLEANUP_CODE    - code to execute at end of triangle
 *
 * The following macro MUST be defined:
 *    RENDER_SPAN(span) - code to write a span of pixels.
 *
 * This code was designed for the origin to be in the lower-left corner.
 *
 * Inspired by triangle rasterizer code written by Allen Akin.  Thanks Allen!
 */


/*
 * This is a bit of a hack, but it's a centralized place to enable floating-
 * point color interpolation when GLchan is actually floating point.
 */



static void simple_textured_triangle(GLcontext *ctx, const SWvertex *v0,
                                 const SWvertex *v1,
                                 const SWvertex *v2 )
{
   typedef struct {
        const SWvertex *v0, *v1;   /* Y(v0) < Y(v1) */
        GLfloat dx;    /* X(v1) - X(v0) */
        GLfloat dy;    /* Y(v1) - Y(v0) */
        GLfixed fdxdy; /* dx/dy in fixed-point */
        GLfixed fsx;   /* first sample point x coord */
        GLfixed fsy;
        GLfloat adjy;  /* adjust from v[0]->fy to fsy, scaled */
        GLint lines;   /* number of lines to be sampled on this edge */
        GLfixed fx0;   /* fixed pt X of lower endpoint */
   } EdgeT;

   EdgeT eMaj, eTop, eBot;
   GLfloat oneOverArea;
   const SWvertex *vMin, *vMid, *vMax;  /* Y(vMin)<=Y(vMid)<=Y(vMax) */
   float bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;
   const GLint snapMask = ~((0x00000800 / (1 << 4)) - 1); /* for x/y coord snapping */
   GLfixed vMin_fx, vMin_fy, vMid_fx, vMid_fy, vMax_fx, vMax_fy;

   struct sw_span span;

   do { (span).primitive = (0x0009); (span).interpMask = (0); (span).arrayMask = (0); (span).start = 0; (span).end = (0); (span).facing = 0; (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays; } while (0);


   printf("%s()\n", __FUNCTION__);
   /*
   printf("  %g, %g, %g\n", v0->win[0], v0->win[1], v0->win[2]);
   printf("  %g, %g, %g\n", v1->win[0], v1->win[1], v1->win[2]);
   printf("  %g, %g, %g\n", v2->win[0], v2->win[1], v2->win[2]);
    */

   /* Compute fixed point x,y coords w/ half-pixel offsets and snapping.
    * And find the order of the 3 vertices along the Y axis.
    */
   {
      const GLfixed fy0 = (((int) ((((v0->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v0->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v0->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy1 = (((int) ((((v1->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v1->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v1->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy2 = (((int) ((((v2->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v2->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v2->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;

      if (fy0 <= fy1) {
         if (fy1 <= fy2) {
            /* y0 <= y1 <= y2 */
            vMin = v0;   vMid = v1;   vMax = v2;
            vMin_fy = fy0;  vMid_fy = fy1;  vMax_fy = fy2;
         }
         else if (fy2 <= fy0) {
            /* y2 <= y0 <= y1 */
            vMin = v2;   vMid = v0;   vMax = v1;
            vMin_fy = fy2;  vMid_fy = fy0;  vMax_fy = fy1;
         }
         else {
            /* y0 <= y2 <= y1 */
            vMin = v0;   vMid = v2;   vMax = v1;
            vMin_fy = fy0;  vMid_fy = fy2;  vMax_fy = fy1;
            bf = -bf;
         }
      }
      else {
         if (fy0 <= fy2) {
            /* y1 <= y0 <= y2 */
            vMin = v1;   vMid = v0;   vMax = v2;
            vMin_fy = fy1;  vMid_fy = fy0;  vMax_fy = fy2;
            bf = -bf;
         }
         else if (fy2 <= fy1) {
            /* y2 <= y1 <= y0 */
            vMin = v2;   vMid = v1;   vMax = v0;
            vMin_fy = fy2;  vMid_fy = fy1;  vMax_fy = fy0;
            bf = -bf;
         }
         else {
            /* y1 <= y2 <= y0 */
            vMin = v1;   vMid = v2;   vMax = v0;
            vMin_fy = fy1;  vMid_fy = fy2;  vMax_fy = fy0;
         }
      }

      /* fixed point X coords */
      vMin_fx = (((int) ((((vMin->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMin->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMin->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMid_fx = (((int) ((((vMid->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMid->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMid->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMax_fx = (((int) ((((vMax->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMax->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMax->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
   }

   /* vertex/edge relationship */
   eMaj.v0 = vMin;   eMaj.v1 = vMax;   /*TODO: .v1's not needed */
   eTop.v0 = vMid;   eTop.v1 = vMax;
   eBot.v0 = vMin;   eBot.v1 = vMid;

   /* compute deltas for each edge:  vertex[upper] - vertex[lower] */
   eMaj.dx = ((vMax_fx - vMin_fx) * (1.0F / 2048.0f));
   eMaj.dy = ((vMax_fy - vMin_fy) * (1.0F / 2048.0f));
   eTop.dx = ((vMax_fx - vMid_fx) * (1.0F / 2048.0f));
   eTop.dy = ((vMax_fy - vMid_fy) * (1.0F / 2048.0f));
   eBot.dx = ((vMid_fx - vMin_fx) * (1.0F / 2048.0f));
   eBot.dy = ((vMid_fy - vMin_fy) * (1.0F / 2048.0f));

   /* compute area, oneOverArea and perform backface culling */
   {
      const GLfloat area = eMaj.dx * eBot.dy - eBot.dx * eMaj.dy;

      /* Do backface culling */
      if (area * bf < 0.0)
         return;

      if (IS_INF_OR_NAN(area) || area == 0.0F)
         return;

      oneOverArea = 1.0F / area;
   }

   ctx->OcclusionResult = 0x1;
   span.facing = ctx->_Facing; /* for 2-sided stencil test */

   /* Edge setup.  For a triangle strip these could be reused... */
   {
      eMaj.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eMaj.lines = (((((vMax_fy - eMaj.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eMaj.lines > 0) {
         GLfloat dxdy = eMaj.dx / eMaj.dy;
         eMaj.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eMaj.adjy = (GLfloat) (eMaj.fsy - vMin_fy);  /* SCALED! */
         eMaj.fx0 = vMin_fx;
         eMaj.fsx = eMaj.fx0 + (GLfixed) (eMaj.adjy * dxdy);
      }
      else {
         return;  /*CULLED*/
      }

      eTop.fsy = (((vMid_fy) + 0x00000800 - 1) & (~0x000007FF));
      eTop.lines = (((((vMax_fy - eTop.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eTop.lines > 0) {
         GLfloat dxdy = eTop.dx / eTop.dy;
         eTop.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eTop.adjy = (GLfloat) (eTop.fsy - vMid_fy); /* SCALED! */
         eTop.fx0 = vMid_fx;
         eTop.fsx = eTop.fx0 + (GLfixed) (eTop.adjy * dxdy);
      }

      eBot.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eBot.lines = (((((vMid_fy - eBot.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eBot.lines > 0) {
         GLfloat dxdy = eBot.dx / eBot.dy;
         eBot.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eBot.adjy = (GLfloat) (eBot.fsy - vMin_fy);  /* SCALED! */
         eBot.fx0 = vMin_fx;
         eBot.fsx = eBot.fx0 + (GLfixed) (eBot.adjy * dxdy);
      }
   }

   /*
    * Conceptually, we view a triangle as two subtriangles
    * separated by a perfectly horizontal line.  The edge that is
    * intersected by this line is one with maximal absolute dy; we
    * call it a ``major'' edge.  The other two edges are the
    * ``top'' edge (for the upper subtriangle) and the ``bottom''
    * edge (for the lower subtriangle).  If either of these two
    * edges is horizontal or very close to horizontal, the
    * corresponding subtriangle might cover zero sample points;
    * we take care to handle such cases, for performance as well
    * as correctness.
    *
    * By stepping rasterization parameters along the major edge,
    * we can avoid recomputing them at the discontinuity where
    * the top and bottom edges meet.  However, this forces us to
    * be able to scan both left-to-right and right-to-left.
    * Also, we must determine whether the major edge is at the
    * left or right side of the triangle.  We do this by
    * computing the magnitude of the cross-product of the major
    * and top edges.  Since this magnitude depends on the sine of
    * the angle between the two edges, its sign tells us whether
    * we turn to the left or to the right when travelling along
    * the major edge to the top edge, and from this we infer
    * whether the major edge is on the left or the right.
    *
    * Serendipitously, this cross-product magnitude is also a
    * value we need to compute the iteration parameter
    * derivatives for the triangle, and it can be used to perform
    * backface culling because its sign tells us whether the
    * triangle is clockwise or counterclockwise.  In this code we
    * refer to it as ``area'' because it's also proportional to
    * the pixel area of the triangle.
    */

   {
      GLint scan_from_left_to_right;  /* true if scanning left-to-right */
      GLfloat dsdx, dsdy;
      GLfloat dtdx, dtdy;

      /*
       * Execute user-supplied setup code
       */
      SWcontext *swrast = ((SWcontext *)ctx->swrast_context); struct gl_texture_object *obj = ctx->Texture.Unit[0].Current2D; const GLint b = obj->BaseLevel; const GLfloat twidth = (GLfloat) obj->Image[b]->Width; const GLfloat theight = (GLfloat) obj->Image[b]->Height; const GLint twidth_log2 = obj->Image[b]->WidthLog2; const GLchan *texture = (const GLchan *) obj->Image[b]->Data; const GLint smask = obj->Image[b]->Width - 1; const GLint tmask = obj->Image[b]->Height - 1; if (!texture) { return; }

      scan_from_left_to_right = (oneOverArea < 0.0F);


      /* compute d?/dx and d?/dy derivatives */
      span.interpMask |= 0x040;
      {
         GLfloat eMaj_ds, eBot_ds;
         eMaj_ds = (vMax->texcoord[0][0] - vMin->texcoord[0][0]) * twidth;
         eBot_ds = (vMid->texcoord[0][0] - vMin->texcoord[0][0]) * twidth;
         dsdx = oneOverArea * (eMaj_ds * eBot.dy - eMaj.dy * eBot_ds);
         span.intTexStep[0] = (((int) ((((dsdx) * 2048.0f) >= 0.0F) ? (((dsdx) * 2048.0f) + 0.5F) : (((dsdx) * 2048.0f) - 0.5F))));
         dsdy = oneOverArea * (eMaj.dx * eBot_ds - eMaj_ds * eBot.dx);
      }
      {
         GLfloat eMaj_dt, eBot_dt;
         eMaj_dt = (vMax->texcoord[0][1] - vMin->texcoord[0][1]) * theight;
         eBot_dt = (vMid->texcoord[0][1] - vMin->texcoord[0][1]) * theight;
         dtdx = oneOverArea * (eMaj_dt * eBot.dy - eMaj.dy * eBot_dt);
         span.intTexStep[1] = (((int) ((((dtdx) * 2048.0f) >= 0.0F) ? (((dtdx) * 2048.0f) + 0.5F) : (((dtdx) * 2048.0f) - 0.5F))));
         dtdy = oneOverArea * (eMaj.dx * eBot_dt - eMaj_dt * eBot.dx);
      }


      /*
       * We always sample at pixel centers.  However, we avoid
       * explicit half-pixel offsets in this code by incorporating
       * the proper offset in each of x and y during the
       * transformation to window coordinates.
       *
       * We also apply the usual rasterization rules to prevent
       * cracks and overlaps.  A pixel is considered inside a
       * subtriangle if it meets all of four conditions: it is on or
       * to the right of the left edge, strictly to the left of the
       * right edge, on or below the top edge, and strictly above
       * the bottom edge.  (Some edges may be degenerate.)
       *
       * The following discussion assumes left-to-right scanning
       * (that is, the major edge is on the left); the right-to-left
       * case is a straightforward variation.
       *
       * We start by finding the half-integral y coordinate that is
       * at or below the top of the triangle.  This gives us the
       * first scan line that could possibly contain pixels that are
       * inside the triangle.
       *
       * Next we creep down the major edge until we reach that y,
       * and compute the corresponding x coordinate on the edge.
       * Then we find the half-integral x that lies on or just
       * inside the edge.  This is the first pixel that might lie in
       * the interior of the triangle.  (We won't know for sure
       * until we check the other edges.)
       *
       * As we rasterize the triangle, we'll step down the major
       * edge.  For each step in y, we'll move an integer number
       * of steps in x.  There are two possible x step sizes, which
       * we'll call the ``inner'' step (guaranteed to land on the
       * edge or inside it) and the ``outer'' step (guaranteed to
       * land on the edge or outside it).  The inner and outer steps
       * differ by one.  During rasterization we maintain an error
       * term that indicates our distance from the true edge, and
       * select either the inner step or the outer step, whichever
       * gets us to the first pixel that falls inside the triangle.
       *
       * All parameters (z, red, etc.) as well as the buffer
       * addresses for color and z have inner and outer step values,
       * so that we can increment them appropriately.  This method
       * eliminates the need to adjust parameters by creeping a
       * sub-pixel amount into the triangle at each scanline.
       */

      {
         int subTriangle;
         GLfixed fx;
         GLfixed fxLeftEdge = 0, fxRightEdge = 0;
         GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
         GLfixed fdxOuter;
         int idxOuter;
         float dxOuter;
         GLfixed fError = 0, fdError = 0;
         float adjx, adjy;
         GLfixed fy;
         GLfixed fs=0, fdsOuter=0, fdsInner;
         GLfixed ft=0, fdtOuter=0, fdtInner;

         for (subTriangle=0; subTriangle<=1; subTriangle++) {
            EdgeT *eLeft, *eRight;
            int setupLeft, setupRight;
            int lines;

            if (subTriangle==0) {
               /* bottom half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eBot;
                  lines = eRight->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
               else {
                  eLeft = &eBot;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
            }
            else {
               /* top half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eTop;
                  lines = eRight->lines;
                  setupLeft = 0;
                  setupRight = 1;
               }
               else {
                  eLeft = &eTop;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 0;
               }
               if (lines == 0)
                  return;
            }

            if (setupLeft && eLeft->lines > 0) {
               const SWvertex *vLower;
               GLfixed fsx = eLeft->fsx;
               fx = (((fsx) + 0x00000800 - 1) & (~0x000007FF));
               fError = fx - fsx - 0x00000800;
               fxLeftEdge = fsx - 1;
               fdxLeftEdge = eLeft->fdxdy;
               fdxOuter = ((fdxLeftEdge - 1) & (~0x000007FF));
               fdError = fdxOuter - fdxLeftEdge + 0x00000800;
               idxOuter = ((fdxOuter) >> 11);
               dxOuter = (float) idxOuter;
               (void) dxOuter;

               fy = eLeft->fsy;
               span.y = ((fy) >> 11);

               adjx = (float)(fx - eLeft->fx0);  /* SCALED! */
               adjy = eLeft->adjy;              /* SCALED! */
               vLower = eLeft->v0;

               /*
                * Now we need the set of parameter (z, color, etc.) values at
                * the point (fx, fy).  This gives us properly-sampled parameter
                * values that we can step from pixel to pixel.  Furthermore,
                * although we might have intermediate results that overflow
                * the normal parameter range when we step temporarily outside
                * the triangle, we shouldn't overflow or underflow for any
                * pixel that's actually inside the triangle.
                */

               {
                  GLfloat s0, t0;
                  s0 = vLower->texcoord[0][0] * twidth;
                  fs = (GLfixed)(s0 * 2048.0f + dsdx * adjx
                                 + dsdy * adjy) + 0x00000400;
                  fdsOuter = (((int) ((((dsdy + dxOuter * dsdx) * 2048.0f) >= 0.0F) ? (((dsdy + dxOuter * dsdx) * 2048.0f) + 0.5F) : (((dsdy + dxOuter * dsdx) * 2048.0f) - 0.5F))));

                  t0 = vLower->texcoord[0][1] * theight;
                  ft = (GLfixed)(t0 * 2048.0f + dtdx * adjx
                                 + dtdy * adjy) + 0x00000400;
                  fdtOuter = (((int) ((((dtdy + dxOuter * dtdx) * 2048.0f) >= 0.0F) ? (((dtdy + dxOuter * dtdx) * 2048.0f) + 0.5F) : (((dtdy + dxOuter * dtdx) * 2048.0f) - 0.5F))));
               }

            } /*if setupLeft*/


            if (setupRight && eRight->lines>0) {
               fxRightEdge = eRight->fsx - 1;
               fdxRightEdge = eRight->fdxdy;
            }

            if (lines==0) {
               continue;
            }


            /* Rasterize setup */
            fdsInner = fdsOuter + span.intTexStep[0];
            fdtInner = fdtOuter + span.intTexStep[1];

            while (lines > 0) {
               /* initialize the span interpolants to the leftmost value */
               /* ff = fixed-pt fragment */
               const GLint right = ((fxRightEdge) >> 11);

               span.x = ((fxLeftEdge) >> 11);

               if (right <= span.x)
                  span.end = 0;
               else
                  span.end = right - span.x;

               span.intTex[0] = fs;
               span.intTex[1] = ft;




               /* This is where we actually generate fragments */
               if (span.end > 0) {
                  GLuint i; span.intTex[0] -= 0x00000400; span.intTex[1] -= 0x00000400; for (i = 0; i < span.end; i++) { GLint s = ((span.intTex[0]) >> 11) & smask; GLint t = ((span.intTex[1]) >> 11) & tmask; GLint pos = (t << twidth_log2) + s; pos = pos + pos + pos; span.array->rgb[i][0] = texture[pos]; span.array->rgb[i][1] = texture[pos+1]; span.array->rgb[i][2] = texture[pos+2]; span.intTex[0] += span.intTexStep[0]; span.intTex[1] += span.intTexStep[1]; } (*swrast->Driver.WriteRGBSpan)(ctx, span.end, span.x, span.y, (const GLchan (*)[3]) span.array->rgb, 0 );;
               }

               /*
                * Advance to the next scan line.  Compute the
                * new edge coordinates, and adjust the
                * pixel-center x coordinate so that it stays
                * on or inside the major edge.
                */
               (span.y)++;
               lines--;

               fxLeftEdge += fdxLeftEdge;
               fxRightEdge += fdxRightEdge;


               fError += fdError;
               if (fError >= 0) {
                  fError -= 0x00000800;
                  fs += fdsOuter;
                  ft += fdtOuter;
               }
               else {
                  fs += fdsInner;
                  ft += fdtInner;
               }
            } /*while lines>0*/

         } /* for subTriangle */

      }
   }
}

/*
 * Render an RGB, GL_DECAL, textured triangle.
 * Interpolate S,T, GL_LESS depth test, w/out mipmapping or
 * perspective correction.
 *
 * No fog.
 */

/*
 * Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom triangle rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated across the triangle:
 *    INTERP_Z        - if defined, interpolate Z values
 *    INTERP_FOG      - if defined, interpolate fog values
 *    INTERP_RGB      - if defined, interpolate RGB values
 *    INTERP_ALPHA    - if defined, interpolate Alpha values (req's INTERP_RGB)
 *    INTERP_SPEC     - if defined, interpolate specular RGB values
 *    INTERP_INDEX    - if defined, interpolate color index values
 *    INTERP_INT_TEX  - if defined, interpolate integer ST texcoords
 *                         (fast, simple 2-D texture mapping)
 *    INTERP_TEX      - if defined, interpolate set 0 float STRQ texcoords
 *                         NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_MULTITEX - if defined, interpolate N units of STRQ texcoords
 *    INTERP_FLOAT_RGBA - if defined, interpolate RGBA with floating point
 *    INTERP_FLOAT_SPEC - if defined, interpolate specular with floating point
 *
 * When one can directly address pixels in the color buffer the following
 * macros can be defined and used to compute pixel addresses during
 * rasterization (see pRow):
 *    PIXEL_TYPE          - the datatype of a pixel (GLubyte, GLushort, GLuint)
 *    BYTES_PER_ROW       - number of bytes per row in the color buffer
 *    PIXEL_ADDRESS(X,Y)  - returns the address of pixel at (X,Y) where
 *                          Y==0 at bottom of screen and increases upward.
 *
 * Similarly, for direct depth buffer access, this type is used for depth
 * buffer addressing:
 *    DEPTH_TYPE          - either GLushort or GLuint
 *
 * Optionally, one may provide one-time setup code per triangle:
 *    SETUP_CODE    - code which is to be executed once per triangle
 *    CLEANUP_CODE    - code to execute at end of triangle
 *
 * The following macro MUST be defined:
 *    RENDER_SPAN(span) - code to write a span of pixels.
 *
 * This code was designed for the origin to be in the lower-left corner.
 *
 * Inspired by triangle rasterizer code written by Allen Akin.  Thanks Allen!
 */


/*
 * This is a bit of a hack, but it's a centralized place to enable floating-
 * point color interpolation when GLchan is actually floating point.
 */



static void simple_z_textured_triangle(GLcontext *ctx, const SWvertex *v0,
                                 const SWvertex *v1,
                                 const SWvertex *v2 )
{
   typedef struct {
        const SWvertex *v0, *v1;   /* Y(v0) < Y(v1) */
        GLfloat dx;    /* X(v1) - X(v0) */
        GLfloat dy;    /* Y(v1) - Y(v0) */
        GLfixed fdxdy; /* dx/dy in fixed-point */
        GLfixed fsx;   /* first sample point x coord */
        GLfixed fsy;
        GLfloat adjy;  /* adjust from v[0]->fy to fsy, scaled */
        GLint lines;   /* number of lines to be sampled on this edge */
        GLfixed fx0;   /* fixed pt X of lower endpoint */
   } EdgeT;

   const GLint depthBits = ctx->Visual.depthBits;
   const GLint fixedToDepthShift = depthBits <= 16 ? FIXED_SHIFT : 0;
   const GLfloat maxDepth = ctx->DepthMaxF;
   EdgeT eMaj, eTop, eBot;
   GLfloat oneOverArea;
   const SWvertex *vMin, *vMid, *vMax;  /* Y(vMin)<=Y(vMid)<=Y(vMax) */
   float bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;
   const GLint snapMask = ~((0x00000800 / (1 << 4)) - 1); /* for x/y coord snapping */
   GLfixed vMin_fx, vMin_fy, vMid_fx, vMid_fy, vMax_fx, vMax_fy;

   struct sw_span span;

   do { (span).primitive = (0x0009); (span).interpMask = (0); (span).arrayMask = (0); (span).start = 0; (span).end = (0); (span).facing = 0; (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays; } while (0);


   printf("%s()\n", __FUNCTION__);
   /*
   printf("  %g, %g, %g\n", v0->win[0], v0->win[1], v0->win[2]);
   printf("  %g, %g, %g\n", v1->win[0], v1->win[1], v1->win[2]);
   printf("  %g, %g, %g\n", v2->win[0], v2->win[1], v2->win[2]);
    */

   /* Compute fixed point x,y coords w/ half-pixel offsets and snapping.
    * And find the order of the 3 vertices along the Y axis.
    */
   {
      const GLfixed fy0 = (((int) ((((v0->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v0->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v0->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy1 = (((int) ((((v1->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v1->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v1->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy2 = (((int) ((((v2->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v2->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v2->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;

      if (fy0 <= fy1) {
         if (fy1 <= fy2) {
            /* y0 <= y1 <= y2 */
            vMin = v0;   vMid = v1;   vMax = v2;
            vMin_fy = fy0;  vMid_fy = fy1;  vMax_fy = fy2;
         }
         else if (fy2 <= fy0) {
            /* y2 <= y0 <= y1 */
            vMin = v2;   vMid = v0;   vMax = v1;
            vMin_fy = fy2;  vMid_fy = fy0;  vMax_fy = fy1;
         }
         else {
            /* y0 <= y2 <= y1 */
            vMin = v0;   vMid = v2;   vMax = v1;
            vMin_fy = fy0;  vMid_fy = fy2;  vMax_fy = fy1;
            bf = -bf;
         }
      }
      else {
         if (fy0 <= fy2) {
            /* y1 <= y0 <= y2 */
            vMin = v1;   vMid = v0;   vMax = v2;
            vMin_fy = fy1;  vMid_fy = fy0;  vMax_fy = fy2;
            bf = -bf;
         }
         else if (fy2 <= fy1) {
            /* y2 <= y1 <= y0 */
            vMin = v2;   vMid = v1;   vMax = v0;
            vMin_fy = fy2;  vMid_fy = fy1;  vMax_fy = fy0;
            bf = -bf;
         }
         else {
            /* y1 <= y2 <= y0 */
            vMin = v1;   vMid = v2;   vMax = v0;
            vMin_fy = fy1;  vMid_fy = fy2;  vMax_fy = fy0;
         }
      }

      /* fixed point X coords */
      vMin_fx = (((int) ((((vMin->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMin->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMin->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMid_fx = (((int) ((((vMid->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMid->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMid->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMax_fx = (((int) ((((vMax->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMax->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMax->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
   }

   /* vertex/edge relationship */
   eMaj.v0 = vMin;   eMaj.v1 = vMax;   /*TODO: .v1's not needed */
   eTop.v0 = vMid;   eTop.v1 = vMax;
   eBot.v0 = vMin;   eBot.v1 = vMid;

   /* compute deltas for each edge:  vertex[upper] - vertex[lower] */
   eMaj.dx = ((vMax_fx - vMin_fx) * (1.0F / 2048.0f));
   eMaj.dy = ((vMax_fy - vMin_fy) * (1.0F / 2048.0f));
   eTop.dx = ((vMax_fx - vMid_fx) * (1.0F / 2048.0f));
   eTop.dy = ((vMax_fy - vMid_fy) * (1.0F / 2048.0f));
   eBot.dx = ((vMid_fx - vMin_fx) * (1.0F / 2048.0f));
   eBot.dy = ((vMid_fy - vMin_fy) * (1.0F / 2048.0f));

   /* compute area, oneOverArea and perform backface culling */
   {
      const GLfloat area = eMaj.dx * eBot.dy - eBot.dx * eMaj.dy;

      /* Do backface culling */
      if (area * bf < 0.0)
         return;

      if (IS_INF_OR_NAN(area) || area == 0.0F)
         return;

      oneOverArea = 1.0F / area;
   }

   ctx->OcclusionResult = 0x1;
   span.facing = ctx->_Facing; /* for 2-sided stencil test */

   /* Edge setup.  For a triangle strip these could be reused... */
   {
      eMaj.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eMaj.lines = (((((vMax_fy - eMaj.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eMaj.lines > 0) {
         GLfloat dxdy = eMaj.dx / eMaj.dy;
         eMaj.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eMaj.adjy = (GLfloat) (eMaj.fsy - vMin_fy);  /* SCALED! */
         eMaj.fx0 = vMin_fx;
         eMaj.fsx = eMaj.fx0 + (GLfixed) (eMaj.adjy * dxdy);
      }
      else {
         return;  /*CULLED*/
      }

      eTop.fsy = (((vMid_fy) + 0x00000800 - 1) & (~0x000007FF));
      eTop.lines = (((((vMax_fy - eTop.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eTop.lines > 0) {
         GLfloat dxdy = eTop.dx / eTop.dy;
         eTop.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eTop.adjy = (GLfloat) (eTop.fsy - vMid_fy); /* SCALED! */
         eTop.fx0 = vMid_fx;
         eTop.fsx = eTop.fx0 + (GLfixed) (eTop.adjy * dxdy);
      }

      eBot.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eBot.lines = (((((vMid_fy - eBot.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eBot.lines > 0) {
         GLfloat dxdy = eBot.dx / eBot.dy;
         eBot.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eBot.adjy = (GLfloat) (eBot.fsy - vMin_fy);  /* SCALED! */
         eBot.fx0 = vMin_fx;
         eBot.fsx = eBot.fx0 + (GLfixed) (eBot.adjy * dxdy);
      }
   }

   /*
    * Conceptually, we view a triangle as two subtriangles
    * separated by a perfectly horizontal line.  The edge that is
    * intersected by this line is one with maximal absolute dy; we
    * call it a ``major'' edge.  The other two edges are the
    * ``top'' edge (for the upper subtriangle) and the ``bottom''
    * edge (for the lower subtriangle).  If either of these two
    * edges is horizontal or very close to horizontal, the
    * corresponding subtriangle might cover zero sample points;
    * we take care to handle such cases, for performance as well
    * as correctness.
    *
    * By stepping rasterization parameters along the major edge,
    * we can avoid recomputing them at the discontinuity where
    * the top and bottom edges meet.  However, this forces us to
    * be able to scan both left-to-right and right-to-left.
    * Also, we must determine whether the major edge is at the
    * left or right side of the triangle.  We do this by
    * computing the magnitude of the cross-product of the major
    * and top edges.  Since this magnitude depends on the sine of
    * the angle between the two edges, its sign tells us whether
    * we turn to the left or to the right when travelling along
    * the major edge to the top edge, and from this we infer
    * whether the major edge is on the left or the right.
    *
    * Serendipitously, this cross-product magnitude is also a
    * value we need to compute the iteration parameter
    * derivatives for the triangle, and it can be used to perform
    * backface culling because its sign tells us whether the
    * triangle is clockwise or counterclockwise.  In this code we
    * refer to it as ``area'' because it's also proportional to
    * the pixel area of the triangle.
    */

   {
      GLint scan_from_left_to_right;  /* true if scanning left-to-right */
      GLfloat dzdx, dzdy;
      GLfloat dsdx, dsdy;
      GLfloat dtdx, dtdy;

      /*
       * Execute user-supplied setup code
       */
      SWcontext *swrast = ((SWcontext *)ctx->swrast_context); struct gl_texture_object *obj = ctx->Texture.Unit[0].Current2D; const GLint b = obj->BaseLevel; const GLfloat twidth = (GLfloat) obj->Image[b]->Width; const GLfloat theight = (GLfloat) obj->Image[b]->Height; const GLint twidth_log2 = obj->Image[b]->WidthLog2; const GLchan *texture = (const GLchan *) obj->Image[b]->Data; const GLint smask = obj->Image[b]->Width - 1; const GLint tmask = obj->Image[b]->Height - 1; if (!texture) { return; }

      scan_from_left_to_right = (oneOverArea < 0.0F);


      /* compute d?/dx and d?/dy derivatives */
      span.interpMask |= 0x008;
      {
         GLfloat eMaj_dz, eBot_dz;
         eMaj_dz = vMax->win[2] - vMin->win[2];
         eBot_dz = vMid->win[2] - vMin->win[2];
         dzdx = oneOverArea * (eMaj_dz * eBot.dy - eMaj.dy * eBot_dz);
         if (dzdx > maxDepth || dzdx < -maxDepth) {
            /* probably a sliver triangle */
            dzdx = 0.0;
            dzdy = 0.0;
         }
         else {
            dzdy = oneOverArea * (eMaj.dx * eBot_dz - eMaj_dz * eBot.dx);
         }
         if (depthBits <= 16)
            span.zStep = (((int) ((((dzdx) * 2048.0f) >= 0.0F) ? (((dzdx) * 2048.0f) + 0.5F) : (((dzdx) * 2048.0f) - 0.5F))));
         else
            span.zStep = (GLint) dzdx;
      }
      span.interpMask |= 0x040;
      {
         GLfloat eMaj_ds, eBot_ds;
         eMaj_ds = (vMax->texcoord[0][0] - vMin->texcoord[0][0]) * twidth;
         eBot_ds = (vMid->texcoord[0][0] - vMin->texcoord[0][0]) * twidth;
         dsdx = oneOverArea * (eMaj_ds * eBot.dy - eMaj.dy * eBot_ds);
         span.intTexStep[0] = (((int) ((((dsdx) * 2048.0f) >= 0.0F) ? (((dsdx) * 2048.0f) + 0.5F) : (((dsdx) * 2048.0f) - 0.5F))));
         dsdy = oneOverArea * (eMaj.dx * eBot_ds - eMaj_ds * eBot.dx);
      }
      {
         GLfloat eMaj_dt, eBot_dt;
         eMaj_dt = (vMax->texcoord[0][1] - vMin->texcoord[0][1]) * theight;
         eBot_dt = (vMid->texcoord[0][1] - vMin->texcoord[0][1]) * theight;
         dtdx = oneOverArea * (eMaj_dt * eBot.dy - eMaj.dy * eBot_dt);
         span.intTexStep[1] = (((int) ((((dtdx) * 2048.0f) >= 0.0F) ? (((dtdx) * 2048.0f) + 0.5F) : (((dtdx) * 2048.0f) - 0.5F))));
         dtdy = oneOverArea * (eMaj.dx * eBot_dt - eMaj_dt * eBot.dx);
      }


      /*
       * We always sample at pixel centers.  However, we avoid
       * explicit half-pixel offsets in this code by incorporating
       * the proper offset in each of x and y during the
       * transformation to window coordinates.
       *
       * We also apply the usual rasterization rules to prevent
       * cracks and overlaps.  A pixel is considered inside a
       * subtriangle if it meets all of four conditions: it is on or
       * to the right of the left edge, strictly to the left of the
       * right edge, on or below the top edge, and strictly above
       * the bottom edge.  (Some edges may be degenerate.)
       *
       * The following discussion assumes left-to-right scanning
       * (that is, the major edge is on the left); the right-to-left
       * case is a straightforward variation.
       *
       * We start by finding the half-integral y coordinate that is
       * at or below the top of the triangle.  This gives us the
       * first scan line that could possibly contain pixels that are
       * inside the triangle.
       *
       * Next we creep down the major edge until we reach that y,
       * and compute the corresponding x coordinate on the edge.
       * Then we find the half-integral x that lies on or just
       * inside the edge.  This is the first pixel that might lie in
       * the interior of the triangle.  (We won't know for sure
       * until we check the other edges.)
       *
       * As we rasterize the triangle, we'll step down the major
       * edge.  For each step in y, we'll move an integer number
       * of steps in x.  There are two possible x step sizes, which
       * we'll call the ``inner'' step (guaranteed to land on the
       * edge or inside it) and the ``outer'' step (guaranteed to
       * land on the edge or outside it).  The inner and outer steps
       * differ by one.  During rasterization we maintain an error
       * term that indicates our distance from the true edge, and
       * select either the inner step or the outer step, whichever
       * gets us to the first pixel that falls inside the triangle.
       *
       * All parameters (z, red, etc.) as well as the buffer
       * addresses for color and z have inner and outer step values,
       * so that we can increment them appropriately.  This method
       * eliminates the need to adjust parameters by creeping a
       * sub-pixel amount into the triangle at each scanline.
       */

      {
         int subTriangle;
         GLfixed fx;
         GLfixed fxLeftEdge = 0, fxRightEdge = 0;
         GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
         GLfixed fdxOuter;
         int idxOuter;
         float dxOuter;
         GLfixed fError = 0, fdError = 0;
         float adjx, adjy;
         GLfixed fy;
         GLushort *zRow = 0;
         int dZRowOuter = 0, dZRowInner;  /* offset in bytes */
         GLfixed fz = 0, fdzOuter = 0, fdzInner;
         GLfixed fs=0, fdsOuter=0, fdsInner;
         GLfixed ft=0, fdtOuter=0, fdtInner;

         for (subTriangle=0; subTriangle<=1; subTriangle++) {
            EdgeT *eLeft, *eRight;
            int setupLeft, setupRight;
            int lines;

            if (subTriangle==0) {
               /* bottom half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eBot;
                  lines = eRight->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
               else {
                  eLeft = &eBot;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
            }
            else {
               /* top half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eTop;
                  lines = eRight->lines;
                  setupLeft = 0;
                  setupRight = 1;
               }
               else {
                  eLeft = &eTop;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 0;
               }
               if (lines == 0)
                  return;
            }

            if (setupLeft && eLeft->lines > 0) {
               const SWvertex *vLower;
               GLfixed fsx = eLeft->fsx;
               fx = (((fsx) + 0x00000800 - 1) & (~0x000007FF));
               fError = fx - fsx - 0x00000800;
               fxLeftEdge = fsx - 1;
               fdxLeftEdge = eLeft->fdxdy;
               fdxOuter = ((fdxLeftEdge - 1) & (~0x000007FF));
               fdError = fdxOuter - fdxLeftEdge + 0x00000800;
               idxOuter = ((fdxOuter) >> 11);
               dxOuter = (float) idxOuter;
               (void) dxOuter;

               fy = eLeft->fsy;
               span.y = ((fy) >> 11);

               adjx = (float)(fx - eLeft->fx0);  /* SCALED! */
               adjy = eLeft->adjy;              /* SCALED! */
               vLower = eLeft->v0;

               /*
                * Now we need the set of parameter (z, color, etc.) values at
                * the point (fx, fy).  This gives us properly-sampled parameter
                * values that we can step from pixel to pixel.  Furthermore,
                * although we might have intermediate results that overflow
                * the normal parameter range when we step temporarily outside
                * the triangle, we shouldn't overflow or underflow for any
                * pixel that's actually inside the triangle.
                */

               {
                  GLfloat z0 = vLower->win[2];
                  if (depthBits <= 16) {
                     /* interpolate fixed-pt values */
                     GLfloat tmp = (z0 * 2048.0f +
                                    dzdx * adjx + dzdy * adjy) + 0x00000400;
                     if (tmp < 0xffffffff / 2)
                        fz = (GLfixed) tmp;
                     else
                        fz = 0xffffffff / 2;
                     fdzOuter = (((int) ((((dzdy + dxOuter * dzdx) * 2048.0f) >= 0.0F) ? (((dzdy + dxOuter * dzdx) * 2048.0f) + 0.5F) : (((dzdy + dxOuter * dzdx) * 2048.0f) - 0.5F))));
                  }
                  else {
                     /* interpolate depth values exactly */
                     fz = (GLint) (z0 + dzdx * ((adjx) * (1.0F / 2048.0f))
                                   + dzdy * ((adjy) * (1.0F / 2048.0f)));
                     fdzOuter = (GLint) (dzdy + dxOuter * dzdx);
                  }
                  zRow = (GLushort *)
                    _mesa_zbuffer_address(ctx, ((fxLeftEdge) >> 11), span.y);
                  dZRowOuter = (ctx->DrawBuffer->Width + idxOuter) * sizeof(GLushort);
               }
               {
                  GLfloat s0, t0;
                  s0 = vLower->texcoord[0][0] * twidth;
                  fs = (GLfixed)(s0 * 2048.0f + dsdx * adjx
                                 + dsdy * adjy) + 0x00000400;
                  fdsOuter = (((int) ((((dsdy + dxOuter * dsdx) * 2048.0f) >= 0.0F) ? (((dsdy + dxOuter * dsdx) * 2048.0f) + 0.5F) : (((dsdy + dxOuter * dsdx) * 2048.0f) - 0.5F))));

                  t0 = vLower->texcoord[0][1] * theight;
                  ft = (GLfixed)(t0 * 2048.0f + dtdx * adjx
                                 + dtdy * adjy) + 0x00000400;
                  fdtOuter = (((int) ((((dtdy + dxOuter * dtdx) * 2048.0f) >= 0.0F) ? (((dtdy + dxOuter * dtdx) * 2048.0f) + 0.5F) : (((dtdy + dxOuter * dtdx) * 2048.0f) - 0.5F))));
               }

            } /*if setupLeft*/


            if (setupRight && eRight->lines>0) {
               fxRightEdge = eRight->fsx - 1;
               fdxRightEdge = eRight->fdxdy;
            }

            if (lines==0) {
               continue;
            }


            /* Rasterize setup */
            dZRowInner = dZRowOuter + sizeof(GLushort);
            fdzInner = fdzOuter + span.zStep;
            fdsInner = fdsOuter + span.intTexStep[0];
            fdtInner = fdtOuter + span.intTexStep[1];

            while (lines > 0) {
               /* initialize the span interpolants to the leftmost value */
               /* ff = fixed-pt fragment */
               const GLint right = ((fxRightEdge) >> 11);

               span.x = ((fxLeftEdge) >> 11);

               if (right <= span.x)
                  span.end = 0;
               else
                  span.end = right - span.x;

               span.z = fz;
               span.intTex[0] = fs;
               span.intTex[1] = ft;




               /* This is where we actually generate fragments */
               if (span.end > 0) {
                  GLuint i; span.intTex[0] -= 0x00000400; span.intTex[1] -= 0x00000400; for (i = 0; i < span.end; i++) { const GLdepth z = ((span.z) >> fixedToDepthShift); if (z < zRow[i]) { GLint s = ((span.intTex[0]) >> 11) & smask; GLint t = ((span.intTex[1]) >> 11) & tmask; GLint pos = (t << twidth_log2) + s; pos = pos + pos + pos; span.array->rgb[i][0] = texture[pos]; span.array->rgb[i][1] = texture[pos+1]; span.array->rgb[i][2] = texture[pos+2]; zRow[i] = z; span.array->mask[i] = 1; } else { span.array->mask[i] = 0; } span.intTex[0] += span.intTexStep[0]; span.intTex[1] += span.intTexStep[1]; span.z += span.zStep; } (*swrast->Driver.WriteRGBSpan)(ctx, span.end, span.x, span.y, (const GLchan (*)[3]) span.array->rgb, span.array->mask );;
               }

               /*
                * Advance to the next scan line.  Compute the
                * new edge coordinates, and adjust the
                * pixel-center x coordinate so that it stays
                * on or inside the major edge.
                */
               (span.y)++;
               lines--;

               fxLeftEdge += fdxLeftEdge;
               fxRightEdge += fdxRightEdge;


               fError += fdError;
               if (fError >= 0) {
                  fError -= 0x00000800;
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowOuter);
                  fz += fdzOuter;
                  fs += fdsOuter;
                  ft += fdtOuter;
               }
               else {
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowInner);
                  fz += fdzInner;
                  fs += fdsInner;
                  ft += fdtInner;
               }
            } /*while lines>0*/

         } /* for subTriangle */

      }
   }
}










struct affine_info
{
   GLenum filter;
   GLenum format;
   GLenum envmode;
   GLint smask, tmask;
   GLint twidth_log2;
   const GLchan *texture;
   GLfixed er, eg, eb, ea;
   GLint tbytesline, tsize;
};


/* This function can handle GL_NEAREST or GL_LINEAR sampling of 2D RGB or RGBA
 * textures with GL_REPLACE, GL_MODULATE, GL_BLEND, GL_DECAL or GL_ADD
 * texture env modes.
 */
static inline void
affine_span(GLcontext *ctx, struct sw_span *span,
            struct affine_info *info)
{
   GLchan sample[4];  /* the filtered texture sample */

   /* Instead of defining a function for each mode, a test is done
    * between the outer and inner loops. This is to reduce code size
    * and complexity. Observe that an optimizing compiler kills
    * unused variables (for instance tf,sf,ti,si in case of GL_NEAREST).
    */










/* shortcuts */






   GLuint i;
   GLchan *dest = span->array->rgba[0];

   span->intTex[0] -= 0x00000400;
   span->intTex[1] -= 0x00000400;
   switch (info->filter) {
   case 0x2600:
      switch (info->format) {
      case 0x1907:
         switch (info->envmode) {
         case 0x2100:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; sample[0] = tex00[0]; sample[1] = tex00[1]; sample[2] = tex00[2]; sample[3] = 255;dest[0] = span->red * (sample[0] + 1u) >> (11 + 8); dest[1] = span->green * (sample[1] + 1u) >> (11 + 8); dest[2] = span->blue * (sample[2] + 1u) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1u) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x2101:
         case 0x1E01:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; sample[0] = tex00[0]; sample[1] = tex00[1]; sample[2] = tex00[2]; sample[3] = 255; dest[0] = sample[0]; dest[1] = sample[1]; dest[2] = sample[2]; dest[3] = ((span->alpha) >> 11);; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x0BE2:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; sample[0] = tex00[0]; sample[1] = tex00[1]; sample[2] = tex00[2]; sample[3] = 255;dest[0] = ((255 - sample[0]) * span->red + (sample[0] + 1) * info->er) >> (11 + 8); dest[1] = ((255 - sample[1]) * span->green + (sample[1] + 1) * info->eg) >> (11 + 8); dest[2] = ((255 - sample[2]) * span->blue + (sample[2] + 1) * info->eb) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x0104:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; sample[0] = tex00[0]; sample[1] = tex00[1]; sample[2] = tex00[2]; sample[3] = 255;{ GLint rSum = ((span->red) >> 11) + (GLint) sample[0]; GLint gSum = ((span->green) >> 11) + (GLint) sample[1]; GLint bSum = ((span->blue) >> 11) + (GLint) sample[2]; dest[0] = ( (rSum)<(255) ? (rSum) : (255) ); dest[1] = ( (gSum)<(255) ? (gSum) : (255) ); dest[2] = ( (bSum)<(255) ? (bSum) : (255) ); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         default:
            _mesa_problem(ctx, "bad tex env mode in SPAN_LINEAR");
            return;
         }
         break;
      case 0x1908:
         switch(info->envmode) {
         case 0x2100:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; { (sample)[0] = (tex00)[0]; (sample)[1] = (tex00)[1]; (sample)[2] = (tex00)[2]; (sample)[3] = (tex00)[3]; };dest[0] = span->red * (sample[0] + 1u) >> (11 + 8); dest[1] = span->green * (sample[1] + 1u) >> (11 + 8); dest[2] = span->blue * (sample[2] + 1u) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1u) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x2101:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; { (sample)[0] = (tex00)[0]; (sample)[1] = (tex00)[1]; (sample)[2] = (tex00)[2]; (sample)[3] = (tex00)[3]; };dest[0] = ((255 - sample[3]) * span->red + ((sample[3] + 1) * sample[0] << 11)) >> (11 + 8); dest[1] = ((255 - sample[3]) * span->green + ((sample[3] + 1) * sample[1] << 11)) >> (11 + 8); dest[2] = ((255 - sample[3]) * span->blue + ((sample[3] + 1) * sample[2] << 11)) >> (11 + 8); dest[3] = ((span->alpha) >> 11); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x0BE2:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; { (sample)[0] = (tex00)[0]; (sample)[1] = (tex00)[1]; (sample)[2] = (tex00)[2]; (sample)[3] = (tex00)[3]; };dest[0] = ((255 - sample[0]) * span->red + (sample[0] + 1) * info->er) >> (11 + 8); dest[1] = ((255 - sample[1]) * span->green + (sample[1] + 1) * info->eg) >> (11 + 8); dest[2] = ((255 - sample[2]) * span->blue + (sample[2] + 1) * info->eb) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x0104:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; { (sample)[0] = (tex00)[0]; (sample)[1] = (tex00)[1]; (sample)[2] = (tex00)[2]; (sample)[3] = (tex00)[3]; };{ GLint rSum = ((span->red) >> 11) + (GLint) sample[0]; GLint gSum = ((span->green) >> 11) + (GLint) sample[1]; GLint bSum = ((span->blue) >> 11) + (GLint) sample[2]; dest[0] = ( (rSum)<(255) ? (rSum) : (255) ); dest[1] = ( (gSum)<(255) ? (gSum) : (255) ); dest[2] = ( (bSum)<(255) ? (bSum) : (255) ); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x1E01:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; { (dest)[0] = (tex00)[0]; (dest)[1] = (tex00)[1]; (dest)[2] = (tex00)[2]; (dest)[3] = (tex00)[3]; }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         default:
            _mesa_problem(ctx, "bad tex env mode (2) in SPAN_LINEAR");
            return;
         }
         break;
      }
      break;

   case 0x2601:
      span->intTex[0] -= 0x00000400;
      span->intTex[1] -= 0x00000400;
      switch (info->format) {
      case 0x1907:
         switch (info->envmode) {
         case 0x2100:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLfixed sf = span->intTex[0] & 0x000007FF; GLfixed tf = span->intTex[1] & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 3; const GLchan *tex11 = tex10 + 3; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = 255;dest[0] = span->red * (sample[0] + 1u) >> (11 + 8); dest[1] = span->green * (sample[1] + 1u) >> (11 + 8); dest[2] = span->blue * (sample[2] + 1u) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1u) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x2101:
         case 0x1E01:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLfixed sf = span->intTex[0] & 0x000007FF; GLfixed tf = span->intTex[1] & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 3; const GLchan *tex11 = tex10 + 3; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = 255;{ (dest)[0] = (sample)[0]; (dest)[1] = (sample)[1]; (dest)[2] = (sample)[2]; (dest)[3] = (sample)[3]; }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x0BE2:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLfixed sf = span->intTex[0] & 0x000007FF; GLfixed tf = span->intTex[1] & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 3; const GLchan *tex11 = tex10 + 3; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = 255;dest[0] = ((255 - sample[0]) * span->red + (sample[0] + 1) * info->er) >> (11 + 8); dest[1] = ((255 - sample[1]) * span->green + (sample[1] + 1) * info->eg) >> (11 + 8); dest[2] = ((255 - sample[2]) * span->blue + (sample[2] + 1) * info->eb) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x0104:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLfixed sf = span->intTex[0] & 0x000007FF; GLfixed tf = span->intTex[1] & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 3; const GLchan *tex11 = tex10 + 3; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = 255;{ GLint rSum = ((span->red) >> 11) + (GLint) sample[0]; GLint gSum = ((span->green) >> 11) + (GLint) sample[1]; GLint bSum = ((span->blue) >> 11) + (GLint) sample[2]; dest[0] = ( (rSum)<(255) ? (rSum) : (255) ); dest[1] = ( (gSum)<(255) ? (gSum) : (255) ); dest[2] = ( (bSum)<(255) ? (bSum) : (255) ); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         default:
            _mesa_problem(ctx, "bad tex env mode (3) in SPAN_LINEAR");
            return;
         }
         break;
      case 0x1908:
         switch (info->envmode) {
         case 0x2100:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLfixed sf = span->intTex[0] & 0x000007FF; GLfixed tf = span->intTex[1] & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 4; const GLchan *tex11 = tex10 + 4; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = (ti * (si * tex00[3] + sf * tex01[3]) + tf * (si * tex10[3] + sf * tex11[3])) >> 2 * 11;dest[0] = span->red * (sample[0] + 1u) >> (11 + 8); dest[1] = span->green * (sample[1] + 1u) >> (11 + 8); dest[2] = span->blue * (sample[2] + 1u) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1u) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x2101:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLfixed sf = span->intTex[0] & 0x000007FF; GLfixed tf = span->intTex[1] & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 4; const GLchan *tex11 = tex10 + 4; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = (ti * (si * tex00[3] + sf * tex01[3]) + tf * (si * tex10[3] + sf * tex11[3])) >> 2 * 11;dest[0] = ((255 - sample[3]) * span->red + ((sample[3] + 1) * sample[0] << 11)) >> (11 + 8); dest[1] = ((255 - sample[3]) * span->green + ((sample[3] + 1) * sample[1] << 11)) >> (11 + 8); dest[2] = ((255 - sample[3]) * span->blue + ((sample[3] + 1) * sample[2] << 11)) >> (11 + 8); dest[3] = ((span->alpha) >> 11); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x0BE2:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLfixed sf = span->intTex[0] & 0x000007FF; GLfixed tf = span->intTex[1] & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 4; const GLchan *tex11 = tex10 + 4; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = (ti * (si * tex00[3] + sf * tex01[3]) + tf * (si * tex10[3] + sf * tex11[3])) >> 2 * 11;dest[0] = ((255 - sample[0]) * span->red + (sample[0] + 1) * info->er) >> (11 + 8); dest[1] = ((255 - sample[1]) * span->green + (sample[1] + 1) * info->eg) >> (11 + 8); dest[2] = ((255 - sample[2]) * span->blue + (sample[2] + 1) * info->eb) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x0104:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLfixed sf = span->intTex[0] & 0x000007FF; GLfixed tf = span->intTex[1] & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 4; const GLchan *tex11 = tex10 + 4; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = (ti * (si * tex00[3] + sf * tex01[3]) + tf * (si * tex10[3] + sf * tex11[3])) >> 2 * 11;{ GLint rSum = ((span->red) >> 11) + (GLint) sample[0]; GLint gSum = ((span->green) >> 11) + (GLint) sample[1]; GLint bSum = ((span->blue) >> 11) + (GLint) sample[2]; dest[0] = ( (rSum)<(255) ? (rSum) : (255) ); dest[1] = ( (gSum)<(255) ? (gSum) : (255) ); dest[2] = ( (bSum)<(255) ? (bSum) : (255) ); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         case 0x1E01:
            for (i = 0; i < span->end; i++) { GLint s = ((span->intTex[0]) >> 11) & info->smask; GLint t = ((span->intTex[1]) >> 11) & info->tmask; GLfixed sf = span->intTex[0] & 0x000007FF; GLfixed tf = span->intTex[1] & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 4; const GLchan *tex11 = tex10 + 4; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = (ti * (si * tex00[3] + sf * tex01[3]) + tf * (si * tex10[3] + sf * tex11[3])) >> 2 * 11;{ (dest)[0] = (sample)[0]; (dest)[1] = (sample)[1]; (dest)[2] = (sample)[2]; (dest)[3] = (sample)[3]; }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; span->intTex[0] += span->intTexStep[0]; span->intTex[1] += span->intTexStep[1]; dest += 4; };
            break;
         default:
            _mesa_problem(ctx, "bad tex env mode (4) in SPAN_LINEAR");
            return;
         }
         break;
      }
      break;
   }
   span->interpMask &= ~0x001;
   ;
   _mesa_write_rgba_span(ctx, span);

}



/*
 * Render an RGB/RGBA textured triangle without perspective correction.
 */



/* $Id: s_tritemp.h,v 1.41 2002/11/13 16:51:02 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.1
 *
 * Copyright (C) 1999-2002  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* $XFree86: xc/extras/Mesa/src/swrast/s_tritemp.h,v 1.2 2002/02/27 21:07:54 tsi Exp $ */

/*
 * Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom triangle rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated across the triangle:
 *    INTERP_Z        - if defined, interpolate Z values
 *    INTERP_FOG      - if defined, interpolate fog values
 *    INTERP_RGB      - if defined, interpolate RGB values
 *    INTERP_ALPHA    - if defined, interpolate Alpha values (req's INTERP_RGB)
 *    INTERP_SPEC     - if defined, interpolate specular RGB values
 *    INTERP_INDEX    - if defined, interpolate color index values
 *    INTERP_INT_TEX  - if defined, interpolate integer ST texcoords
 *                         (fast, simple 2-D texture mapping)
 *    INTERP_TEX      - if defined, interpolate set 0 float STRQ texcoords
 *                         NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_MULTITEX - if defined, interpolate N units of STRQ texcoords
 *    INTERP_FLOAT_RGBA - if defined, interpolate RGBA with floating point
 *    INTERP_FLOAT_SPEC - if defined, interpolate specular with floating point
 *
 * When one can directly address pixels in the color buffer the following
 * macros can be defined and used to compute pixel addresses during
 * rasterization (see pRow):
 *    PIXEL_TYPE          - the datatype of a pixel (GLubyte, GLushort, GLuint)
 *    BYTES_PER_ROW       - number of bytes per row in the color buffer
 *    PIXEL_ADDRESS(X,Y)  - returns the address of pixel at (X,Y) where
 *                          Y==0 at bottom of screen and increases upward.
 *
 * Similarly, for direct depth buffer access, this type is used for depth
 * buffer addressing:
 *    DEPTH_TYPE          - either GLushort or GLuint
 *
 * Optionally, one may provide one-time setup code per triangle:
 *    SETUP_CODE    - code which is to be executed once per triangle
 *    CLEANUP_CODE    - code to execute at end of triangle
 *
 * The following macro MUST be defined:
 *    RENDER_SPAN(span) - code to write a span of pixels.
 *
 * This code was designed for the origin to be in the lower-left corner.
 *
 * Inspired by triangle rasterizer code written by Allen Akin.  Thanks Allen!
 */


/*
 * This is a bit of a hack, but it's a centralized place to enable floating-
 * point color interpolation when GLchan is actually floating point.
 */



static void affine_textured_triangle(GLcontext *ctx, const SWvertex *v0,
                                 const SWvertex *v1,
                                 const SWvertex *v2 )
{
   typedef struct {
        const SWvertex *v0, *v1;   /* Y(v0) < Y(v1) */
        GLfloat dx;    /* X(v1) - X(v0) */
        GLfloat dy;    /* Y(v1) - Y(v0) */
        GLfixed fdxdy; /* dx/dy in fixed-point */
        GLfixed fsx;   /* first sample point x coord */
        GLfixed fsy;
        GLfloat adjy;  /* adjust from v[0]->fy to fsy, scaled */
        GLint lines;   /* number of lines to be sampled on this edge */
        GLfixed fx0;   /* fixed pt X of lower endpoint */
   } EdgeT;

   const GLint depthBits = ctx->Visual.depthBits;
   const GLfloat maxDepth = ctx->DepthMaxF;
   EdgeT eMaj, eTop, eBot;
   GLfloat oneOverArea;
   const SWvertex *vMin, *vMid, *vMax;  /* Y(vMin)<=Y(vMid)<=Y(vMax) */
   float bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;
   const GLint snapMask = ~((0x00000800 / (1 << 4)) - 1); /* for x/y coord snapping */
   GLfixed vMin_fx, vMin_fy, vMid_fx, vMid_fy, vMax_fx, vMax_fy;

   struct sw_span span;

   do { (span).primitive = (0x0009); (span).interpMask = (0); (span).arrayMask = (0); (span).start = 0; (span).end = (0); (span).facing = 0; (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays; } while (0);


   printf("%s()\n", __FUNCTION__);
   /*
   printf("  %g, %g, %g\n", v0->win[0], v0->win[1], v0->win[2]);
   printf("  %g, %g, %g\n", v1->win[0], v1->win[1], v1->win[2]);
   printf("  %g, %g, %g\n", v2->win[0], v2->win[1], v2->win[2]);
    */

   /* Compute fixed point x,y coords w/ half-pixel offsets and snapping.
    * And find the order of the 3 vertices along the Y axis.
    */
   {
      const GLfixed fy0 = (((int) ((((v0->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v0->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v0->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy1 = (((int) ((((v1->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v1->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v1->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy2 = (((int) ((((v2->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v2->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v2->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;

      if (fy0 <= fy1) {
         if (fy1 <= fy2) {
            /* y0 <= y1 <= y2 */
            vMin = v0;   vMid = v1;   vMax = v2;
            vMin_fy = fy0;  vMid_fy = fy1;  vMax_fy = fy2;
         }
         else if (fy2 <= fy0) {
            /* y2 <= y0 <= y1 */
            vMin = v2;   vMid = v0;   vMax = v1;
            vMin_fy = fy2;  vMid_fy = fy0;  vMax_fy = fy1;
         }
         else {
            /* y0 <= y2 <= y1 */
            vMin = v0;   vMid = v2;   vMax = v1;
            vMin_fy = fy0;  vMid_fy = fy2;  vMax_fy = fy1;
            bf = -bf;
         }
      }
      else {
         if (fy0 <= fy2) {
            /* y1 <= y0 <= y2 */
            vMin = v1;   vMid = v0;   vMax = v2;
            vMin_fy = fy1;  vMid_fy = fy0;  vMax_fy = fy2;
            bf = -bf;
         }
         else if (fy2 <= fy1) {
            /* y2 <= y1 <= y0 */
            vMin = v2;   vMid = v1;   vMax = v0;
            vMin_fy = fy2;  vMid_fy = fy1;  vMax_fy = fy0;
            bf = -bf;
         }
         else {
            /* y1 <= y2 <= y0 */
            vMin = v1;   vMid = v2;   vMax = v0;
            vMin_fy = fy1;  vMid_fy = fy2;  vMax_fy = fy0;
         }
      }

      /* fixed point X coords */
      vMin_fx = (((int) ((((vMin->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMin->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMin->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMid_fx = (((int) ((((vMid->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMid->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMid->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMax_fx = (((int) ((((vMax->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMax->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMax->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
   }

   /* vertex/edge relationship */
   eMaj.v0 = vMin;   eMaj.v1 = vMax;   /*TODO: .v1's not needed */
   eTop.v0 = vMid;   eTop.v1 = vMax;
   eBot.v0 = vMin;   eBot.v1 = vMid;

   /* compute deltas for each edge:  vertex[upper] - vertex[lower] */
   eMaj.dx = ((vMax_fx - vMin_fx) * (1.0F / 2048.0f));
   eMaj.dy = ((vMax_fy - vMin_fy) * (1.0F / 2048.0f));
   eTop.dx = ((vMax_fx - vMid_fx) * (1.0F / 2048.0f));
   eTop.dy = ((vMax_fy - vMid_fy) * (1.0F / 2048.0f));
   eBot.dx = ((vMid_fx - vMin_fx) * (1.0F / 2048.0f));
   eBot.dy = ((vMid_fy - vMin_fy) * (1.0F / 2048.0f));

   /* compute area, oneOverArea and perform backface culling */
   {
      const GLfloat area = eMaj.dx * eBot.dy - eBot.dx * eMaj.dy;

      /* Do backface culling */
      if (area * bf < 0.0)
         return;

      if (IS_INF_OR_NAN(area) || area == 0.0F)
         return;

      oneOverArea = 1.0F / area;
   }

   ctx->OcclusionResult = 0x1;
   span.facing = ctx->_Facing; /* for 2-sided stencil test */

   /* Edge setup.  For a triangle strip these could be reused... */
   {
      eMaj.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eMaj.lines = (((((vMax_fy - eMaj.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eMaj.lines > 0) {
         GLfloat dxdy = eMaj.dx / eMaj.dy;
         eMaj.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eMaj.adjy = (GLfloat) (eMaj.fsy - vMin_fy);  /* SCALED! */
         eMaj.fx0 = vMin_fx;
         eMaj.fsx = eMaj.fx0 + (GLfixed) (eMaj.adjy * dxdy);
      }
      else {
         return;  /*CULLED*/
      }

      eTop.fsy = (((vMid_fy) + 0x00000800 - 1) & (~0x000007FF));
      eTop.lines = (((((vMax_fy - eTop.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eTop.lines > 0) {
         GLfloat dxdy = eTop.dx / eTop.dy;
         eTop.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eTop.adjy = (GLfloat) (eTop.fsy - vMid_fy); /* SCALED! */
         eTop.fx0 = vMid_fx;
         eTop.fsx = eTop.fx0 + (GLfixed) (eTop.adjy * dxdy);
      }

      eBot.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eBot.lines = (((((vMid_fy - eBot.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eBot.lines > 0) {
         GLfloat dxdy = eBot.dx / eBot.dy;
         eBot.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eBot.adjy = (GLfloat) (eBot.fsy - vMin_fy);  /* SCALED! */
         eBot.fx0 = vMin_fx;
         eBot.fsx = eBot.fx0 + (GLfixed) (eBot.adjy * dxdy);
      }
   }

   /*
    * Conceptually, we view a triangle as two subtriangles
    * separated by a perfectly horizontal line.  The edge that is
    * intersected by this line is one with maximal absolute dy; we
    * call it a ``major'' edge.  The other two edges are the
    * ``top'' edge (for the upper subtriangle) and the ``bottom''
    * edge (for the lower subtriangle).  If either of these two
    * edges is horizontal or very close to horizontal, the
    * corresponding subtriangle might cover zero sample points;
    * we take care to handle such cases, for performance as well
    * as correctness.
    *
    * By stepping rasterization parameters along the major edge,
    * we can avoid recomputing them at the discontinuity where
    * the top and bottom edges meet.  However, this forces us to
    * be able to scan both left-to-right and right-to-left.
    * Also, we must determine whether the major edge is at the
    * left or right side of the triangle.  We do this by
    * computing the magnitude of the cross-product of the major
    * and top edges.  Since this magnitude depends on the sine of
    * the angle between the two edges, its sign tells us whether
    * we turn to the left or to the right when travelling along
    * the major edge to the top edge, and from this we infer
    * whether the major edge is on the left or the right.
    *
    * Serendipitously, this cross-product magnitude is also a
    * value we need to compute the iteration parameter
    * derivatives for the triangle, and it can be used to perform
    * backface culling because its sign tells us whether the
    * triangle is clockwise or counterclockwise.  In this code we
    * refer to it as ``area'' because it's also proportional to
    * the pixel area of the triangle.
    */

   {
      GLint scan_from_left_to_right;  /* true if scanning left-to-right */
      GLfloat dzdx, dzdy;
      GLfloat dfogdy;
      GLfloat drdx, drdy;
      GLfloat dgdx, dgdy;
      GLfloat dbdx, dbdy;
      GLfloat dadx, dady;
      GLfloat dsdx, dsdy;
      GLfloat dtdx, dtdy;

      /*
       * Execute user-supplied setup code
       */
      struct affine_info info; struct gl_texture_unit *unit = ctx->Texture.Unit+0; struct gl_texture_object *obj = unit->Current2D; const GLint b = obj->BaseLevel; const GLfloat twidth = (GLfloat) obj->Image[b]->Width; const GLfloat theight = (GLfloat) obj->Image[b]->Height; info.texture = (const GLchan *) obj->Image[b]->Data; info.twidth_log2 = obj->Image[b]->WidthLog2; info.smask = obj->Image[b]->Width - 1; info.tmask = obj->Image[b]->Height - 1; info.format = obj->Image[b]->Format; info.filter = obj->MinFilter; info.envmode = unit->EnvMode; span.arrayMask |= 0x001; if (info.envmode == 0x0BE2) { info.er = (((int) ((((unit->EnvColor[0] * 255.0F) * 2048.0f) >= 0.0F) ? (((unit->EnvColor[0] * 255.0F) * 2048.0f) + 0.5F) : (((unit->EnvColor[0] * 255.0F) * 2048.0f) - 0.5F)))); info.eg = (((int) ((((unit->EnvColor[1] * 255.0F) * 2048.0f) >= 0.0F) ? (((unit->EnvColor[1] * 255.0F) * 2048.0f) + 0.5F) : (((unit->EnvColor[1] * 255.0F) * 2048.0f) - 0.5F)))); info.eb = (((int) ((((unit->EnvColor[2] * 255.0F) * 2048.0f) >= 0.0F) ? (((unit->EnvColor[2] * 255.0F) * 2048.0f) + 0.5F) : (((unit->EnvColor[2] * 255.0F) * 2048.0f) - 0.5F)))); info.ea = (((int) ((((unit->EnvColor[3] * 255.0F) * 2048.0f) >= 0.0F) ? (((unit->EnvColor[3] * 255.0F) * 2048.0f) + 0.5F) : (((unit->EnvColor[3] * 255.0F) * 2048.0f) - 0.5F)))); } if (!info.texture) { return; } switch (info.format) { case 0x1906: case 0x1909: case 0x8049: info.tbytesline = obj->Image[b]->Width; break; case 0x190A: info.tbytesline = obj->Image[b]->Width * 2; break; case 0x1907: info.tbytesline = obj->Image[b]->Width * 3; break; case 0x1908: info.tbytesline = obj->Image[b]->Width * 4; break; default: _mesa_problem(0, "Bad texture format in affine_texture_triangle"); return; } info.tsize = obj->Image[b]->Height * info.tbytesline;

      scan_from_left_to_right = (oneOverArea < 0.0F);


      /* compute d?/dx and d?/dy derivatives */
      span.interpMask |= 0x008;
      {
         GLfloat eMaj_dz, eBot_dz;
         eMaj_dz = vMax->win[2] - vMin->win[2];
         eBot_dz = vMid->win[2] - vMin->win[2];
         dzdx = oneOverArea * (eMaj_dz * eBot.dy - eMaj.dy * eBot_dz);
         if (dzdx > maxDepth || dzdx < -maxDepth) {
            /* probably a sliver triangle */
            dzdx = 0.0;
            dzdy = 0.0;
         }
         else {
            dzdy = oneOverArea * (eMaj.dx * eBot_dz - eMaj_dz * eBot.dx);
         }
         if (depthBits <= 16)
            span.zStep = (((int) ((((dzdx) * 2048.0f) >= 0.0F) ? (((dzdx) * 2048.0f) + 0.5F) : (((dzdx) * 2048.0f) - 0.5F))));
         else
            span.zStep = (GLint) dzdx;
      }
      span.interpMask |= 0x010;
      {
         const GLfloat eMaj_dfog = vMax->fog - vMin->fog;
         const GLfloat eBot_dfog = vMid->fog - vMin->fog;
         span.fogStep = oneOverArea * (eMaj_dfog * eBot.dy - eMaj.dy * eBot_dfog);
         dfogdy = oneOverArea * (eMaj.dx * eBot_dfog - eMaj_dfog * eBot.dx);
      }
      span.interpMask |= 0x001;
      if (ctx->Light.ShadeModel == 0x1D01) {
         GLfloat eMaj_dr, eBot_dr;
         GLfloat eMaj_dg, eBot_dg;
         GLfloat eMaj_db, eBot_db;
         GLfloat eMaj_da, eBot_da;
         eMaj_dr = (GLfloat) ((GLint) vMax->color[0] -
                             (GLint) vMin->color[0]);
         eBot_dr = (GLfloat) ((GLint) vMid->color[0] -
                             (GLint) vMin->color[0]);
         drdx = oneOverArea * (eMaj_dr * eBot.dy - eMaj.dy * eBot_dr);
         span.redStep = (((int) ((((drdx) * 2048.0f) >= 0.0F) ? (((drdx) * 2048.0f) + 0.5F) : (((drdx) * 2048.0f) - 0.5F))));
         drdy = oneOverArea * (eMaj.dx * eBot_dr - eMaj_dr * eBot.dx);
         eMaj_dg = (GLfloat) ((GLint) vMax->color[1] -
                             (GLint) vMin->color[1]);
         eBot_dg = (GLfloat) ((GLint) vMid->color[1] -
                             (GLint) vMin->color[1]);
         dgdx = oneOverArea * (eMaj_dg * eBot.dy - eMaj.dy * eBot_dg);
         span.greenStep = (((int) ((((dgdx) * 2048.0f) >= 0.0F) ? (((dgdx) * 2048.0f) + 0.5F) : (((dgdx) * 2048.0f) - 0.5F))));
         dgdy = oneOverArea * (eMaj.dx * eBot_dg - eMaj_dg * eBot.dx);
         eMaj_db = (GLfloat) ((GLint) vMax->color[2] -
                             (GLint) vMin->color[2]);
         eBot_db = (GLfloat) ((GLint) vMid->color[2] -
                             (GLint) vMin->color[2]);
         dbdx = oneOverArea * (eMaj_db * eBot.dy - eMaj.dy * eBot_db);
         span.blueStep = (((int) ((((dbdx) * 2048.0f) >= 0.0F) ? (((dbdx) * 2048.0f) + 0.5F) : (((dbdx) * 2048.0f) - 0.5F))));
         dbdy = oneOverArea * (eMaj.dx * eBot_db - eMaj_db * eBot.dx);
         eMaj_da = (GLfloat) ((GLint) vMax->color[3] -
                             (GLint) vMin->color[3]);
         eBot_da = (GLfloat) ((GLint) vMid->color[3] -
                             (GLint) vMin->color[3]);
         dadx = oneOverArea * (eMaj_da * eBot.dy - eMaj.dy * eBot_da);
         span.alphaStep = (((int) ((((dadx) * 2048.0f) >= 0.0F) ? (((dadx) * 2048.0f) + 0.5F) : (((dadx) * 2048.0f) - 0.5F))));
         dady = oneOverArea * (eMaj.dx * eBot_da - eMaj_da * eBot.dx);
      }
      else {
         ;
         span.interpMask |= 0x200;
         drdx = drdy = 0.0F;
         dgdx = dgdy = 0.0F;
         dbdx = dbdy = 0.0F;
         span.redStep = 0;
         span.greenStep = 0;
         span.blueStep = 0;
         dadx = dady = 0.0F;
         span.alphaStep = 0;
      }
      span.interpMask |= 0x040;
      {
         GLfloat eMaj_ds, eBot_ds;
         eMaj_ds = (vMax->texcoord[0][0] - vMin->texcoord[0][0]) * twidth;
         eBot_ds = (vMid->texcoord[0][0] - vMin->texcoord[0][0]) * twidth;
         dsdx = oneOverArea * (eMaj_ds * eBot.dy - eMaj.dy * eBot_ds);
         span.intTexStep[0] = (((int) ((((dsdx) * 2048.0f) >= 0.0F) ? (((dsdx) * 2048.0f) + 0.5F) : (((dsdx) * 2048.0f) - 0.5F))));
         dsdy = oneOverArea * (eMaj.dx * eBot_ds - eMaj_ds * eBot.dx);
      }
      {
         GLfloat eMaj_dt, eBot_dt;
         eMaj_dt = (vMax->texcoord[0][1] - vMin->texcoord[0][1]) * theight;
         eBot_dt = (vMid->texcoord[0][1] - vMin->texcoord[0][1]) * theight;
         dtdx = oneOverArea * (eMaj_dt * eBot.dy - eMaj.dy * eBot_dt);
         span.intTexStep[1] = (((int) ((((dtdx) * 2048.0f) >= 0.0F) ? (((dtdx) * 2048.0f) + 0.5F) : (((dtdx) * 2048.0f) - 0.5F))));
         dtdy = oneOverArea * (eMaj.dx * eBot_dt - eMaj_dt * eBot.dx);
      }


      /*
       * We always sample at pixel centers.  However, we avoid
       * explicit half-pixel offsets in this code by incorporating
       * the proper offset in each of x and y during the
       * transformation to window coordinates.
       *
       * We also apply the usual rasterization rules to prevent
       * cracks and overlaps.  A pixel is considered inside a
       * subtriangle if it meets all of four conditions: it is on or
       * to the right of the left edge, strictly to the left of the
       * right edge, on or below the top edge, and strictly above
       * the bottom edge.  (Some edges may be degenerate.)
       *
       * The following discussion assumes left-to-right scanning
       * (that is, the major edge is on the left); the right-to-left
       * case is a straightforward variation.
       *
       * We start by finding the half-integral y coordinate that is
       * at or below the top of the triangle.  This gives us the
       * first scan line that could possibly contain pixels that are
       * inside the triangle.
       *
       * Next we creep down the major edge until we reach that y,
       * and compute the corresponding x coordinate on the edge.
       * Then we find the half-integral x that lies on or just
       * inside the edge.  This is the first pixel that might lie in
       * the interior of the triangle.  (We won't know for sure
       * until we check the other edges.)
       *
       * As we rasterize the triangle, we'll step down the major
       * edge.  For each step in y, we'll move an integer number
       * of steps in x.  There are two possible x step sizes, which
       * we'll call the ``inner'' step (guaranteed to land on the
       * edge or inside it) and the ``outer'' step (guaranteed to
       * land on the edge or outside it).  The inner and outer steps
       * differ by one.  During rasterization we maintain an error
       * term that indicates our distance from the true edge, and
       * select either the inner step or the outer step, whichever
       * gets us to the first pixel that falls inside the triangle.
       *
       * All parameters (z, red, etc.) as well as the buffer
       * addresses for color and z have inner and outer step values,
       * so that we can increment them appropriately.  This method
       * eliminates the need to adjust parameters by creeping a
       * sub-pixel amount into the triangle at each scanline.
       */

      {
         int subTriangle;
         GLfixed fx;
         GLfixed fxLeftEdge = 0, fxRightEdge = 0;
         GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
         GLfixed fdxOuter;
         int idxOuter;
         float dxOuter;
         GLfixed fError = 0, fdError = 0;
         float adjx, adjy;
         GLfixed fy;
         GLushort *zRow = 0;
         int dZRowOuter = 0, dZRowInner;  /* offset in bytes */
         GLfixed fz = 0, fdzOuter = 0, fdzInner;
         GLfloat fogLeft = 0, dfogOuter = 0, dfogInner;
         GLfixed fr = 0, fdrOuter = 0, fdrInner;
         GLfixed fg = 0, fdgOuter = 0, fdgInner;
         GLfixed fb = 0, fdbOuter = 0, fdbInner;
         GLfixed fa = 0, fdaOuter = 0, fdaInner;
         GLfixed fs=0, fdsOuter=0, fdsInner;
         GLfixed ft=0, fdtOuter=0, fdtInner;

         for (subTriangle=0; subTriangle<=1; subTriangle++) {
            EdgeT *eLeft, *eRight;
            int setupLeft, setupRight;
            int lines;

            if (subTriangle==0) {
               /* bottom half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eBot;
                  lines = eRight->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
               else {
                  eLeft = &eBot;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
            }
            else {
               /* top half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eTop;
                  lines = eRight->lines;
                  setupLeft = 0;
                  setupRight = 1;
               }
               else {
                  eLeft = &eTop;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 0;
               }
               if (lines == 0)
                  return;
            }

            if (setupLeft && eLeft->lines > 0) {
               const SWvertex *vLower;
               GLfixed fsx = eLeft->fsx;
               fx = (((fsx) + 0x00000800 - 1) & (~0x000007FF));
               fError = fx - fsx - 0x00000800;
               fxLeftEdge = fsx - 1;
               fdxLeftEdge = eLeft->fdxdy;
               fdxOuter = ((fdxLeftEdge - 1) & (~0x000007FF));
               fdError = fdxOuter - fdxLeftEdge + 0x00000800;
               idxOuter = ((fdxOuter) >> 11);
               dxOuter = (float) idxOuter;
               (void) dxOuter;

               fy = eLeft->fsy;
               span.y = ((fy) >> 11);

               adjx = (float)(fx - eLeft->fx0);  /* SCALED! */
               adjy = eLeft->adjy;              /* SCALED! */
               vLower = eLeft->v0;

               /*
                * Now we need the set of parameter (z, color, etc.) values at
                * the point (fx, fy).  This gives us properly-sampled parameter
                * values that we can step from pixel to pixel.  Furthermore,
                * although we might have intermediate results that overflow
                * the normal parameter range when we step temporarily outside
                * the triangle, we shouldn't overflow or underflow for any
                * pixel that's actually inside the triangle.
                */

               {
                  GLfloat z0 = vLower->win[2];
                  if (depthBits <= 16) {
                     /* interpolate fixed-pt values */
                     GLfloat tmp = (z0 * 2048.0f +
                                    dzdx * adjx + dzdy * adjy) + 0x00000400;
                     if (tmp < 0xffffffff / 2)
                        fz = (GLfixed) tmp;
                     else
                        fz = 0xffffffff / 2;
                     fdzOuter = (((int) ((((dzdy + dxOuter * dzdx) * 2048.0f) >= 0.0F) ? (((dzdy + dxOuter * dzdx) * 2048.0f) + 0.5F) : (((dzdy + dxOuter * dzdx) * 2048.0f) - 0.5F))));
                  }
                  else {
                     /* interpolate depth values exactly */
                     fz = (GLint) (z0 + dzdx * ((adjx) * (1.0F / 2048.0f))
                                   + dzdy * ((adjy) * (1.0F / 2048.0f)));
                     fdzOuter = (GLint) (dzdy + dxOuter * dzdx);
                  }
                  zRow = (GLushort *)
                    _mesa_zbuffer_address(ctx, ((fxLeftEdge) >> 11), span.y);
                  dZRowOuter = (ctx->DrawBuffer->Width + idxOuter) * sizeof(GLushort);
               }
               fogLeft = vLower->fog + (span.fogStep * adjx + dfogdy * adjy)
                                       * (1.0F/2048.0f);
               dfogOuter = dfogdy + dxOuter * span.fogStep;
               if (ctx->Light.ShadeModel == 0x1D01) {
                  fr = (GLfixed) (((vLower->color[0]) << 11)
                                   + drdx * adjx + drdy * adjy) + 0x00000400;
                  fdrOuter = (((int) ((((drdy + dxOuter * drdx) * 2048.0f) >= 0.0F) ? (((drdy + dxOuter * drdx) * 2048.0f) + 0.5F) : (((drdy + dxOuter * drdx) * 2048.0f) - 0.5F))));
                  fg = (GLfixed) (((vLower->color[1]) << 11)
                                   + dgdx * adjx + dgdy * adjy) + 0x00000400;
                  fdgOuter = (((int) ((((dgdy + dxOuter * dgdx) * 2048.0f) >= 0.0F) ? (((dgdy + dxOuter * dgdx) * 2048.0f) + 0.5F) : (((dgdy + dxOuter * dgdx) * 2048.0f) - 0.5F))));
                  fb = (GLfixed) (((vLower->color[2]) << 11)
                                    + dbdx * adjx + dbdy * adjy) + 0x00000400;
                  fdbOuter = (((int) ((((dbdy + dxOuter * dbdx) * 2048.0f) >= 0.0F) ? (((dbdy + dxOuter * dbdx) * 2048.0f) + 0.5F) : (((dbdy + dxOuter * dbdx) * 2048.0f) - 0.5F))));
                  fa = (GLfixed) (((vLower->color[3]) << 11)
                                   + dadx * adjx + dady * adjy) + 0x00000400;
                  fdaOuter = (((int) ((((dady + dxOuter * dadx) * 2048.0f) >= 0.0F) ? (((dady + dxOuter * dadx) * 2048.0f) + 0.5F) : (((dady + dxOuter * dadx) * 2048.0f) - 0.5F))));
               }
               else {
                  ;
                  fr = ((v2->color[0]) << 11);
                  fg = ((v2->color[1]) << 11);
                  fb = ((v2->color[2]) << 11);
                  fdrOuter = fdgOuter = fdbOuter = 0;
                  fa =  ((v2->color[3]) << 11);
                  fdaOuter = 0;
               }
               {
                  GLfloat s0, t0;
                  s0 = vLower->texcoord[0][0] * twidth;
                  fs = (GLfixed)(s0 * 2048.0f + dsdx * adjx
                                 + dsdy * adjy) + 0x00000400;
                  fdsOuter = (((int) ((((dsdy + dxOuter * dsdx) * 2048.0f) >= 0.0F) ? (((dsdy + dxOuter * dsdx) * 2048.0f) + 0.5F) : (((dsdy + dxOuter * dsdx) * 2048.0f) - 0.5F))));

                  t0 = vLower->texcoord[0][1] * theight;
                  ft = (GLfixed)(t0 * 2048.0f + dtdx * adjx
                                 + dtdy * adjy) + 0x00000400;
                  fdtOuter = (((int) ((((dtdy + dxOuter * dtdx) * 2048.0f) >= 0.0F) ? (((dtdy + dxOuter * dtdx) * 2048.0f) + 0.5F) : (((dtdy + dxOuter * dtdx) * 2048.0f) - 0.5F))));
               }

            } /*if setupLeft*/


            if (setupRight && eRight->lines>0) {
               fxRightEdge = eRight->fsx - 1;
               fdxRightEdge = eRight->fdxdy;
            }

            if (lines==0) {
               continue;
            }


            /* Rasterize setup */
            dZRowInner = dZRowOuter + sizeof(GLushort);
            fdzInner = fdzOuter + span.zStep;
            dfogInner = dfogOuter + span.fogStep;
            fdrInner = fdrOuter + span.redStep;
            fdgInner = fdgOuter + span.greenStep;
            fdbInner = fdbOuter + span.blueStep;
            fdaInner = fdaOuter + span.alphaStep;
            fdsInner = fdsOuter + span.intTexStep[0];
            fdtInner = fdtOuter + span.intTexStep[1];

            while (lines > 0) {
               /* initialize the span interpolants to the leftmost value */
               /* ff = fixed-pt fragment */
               const GLint right = ((fxRightEdge) >> 11);

               span.x = ((fxLeftEdge) >> 11);

               if (right <= span.x)
                  span.end = 0;
               else
                  span.end = right - span.x;

               span.z = fz;
               span.fog = fogLeft;
               span.red = fr;
               span.green = fg;
               span.blue = fb;
               span.alpha = fa;
               span.intTex[0] = fs;
               span.intTex[1] = ft;



               {
                  /* need this to accomodate round-off errors */
                  const GLint len = right - span.x - 1;
                  GLfixed ffrend = span.red + len * span.redStep;
                  GLfixed ffgend = span.green + len * span.greenStep;
                  GLfixed ffbend = span.blue + len * span.blueStep;
                  if (ffrend < 0) {
                     span.red -= ffrend;
                     if (span.red < 0)
                        span.red = 0;
                  }
                  if (ffgend < 0) {
                     span.green -= ffgend;
                     if (span.green < 0)
                        span.green = 0;
                  }
                  if (ffbend < 0) {
                     span.blue -= ffbend;
                     if (span.blue < 0)
                        span.blue = 0;
                  }
               }
               {
                  const GLint len = right - span.x - 1;
                  GLfixed ffaend = span.alpha + len * span.alphaStep;
                  if (ffaend < 0) {
                     span.alpha -= ffaend;
                     if (span.alpha < 0)
                        span.alpha = 0;
                  }
               }

               /* This is where we actually generate fragments */
               if (span.end > 0) {
                  affine_span(ctx, &span, &info);;
               }

               /*
                * Advance to the next scan line.  Compute the
                * new edge coordinates, and adjust the
                * pixel-center x coordinate so that it stays
                * on or inside the major edge.
                */
               (span.y)++;
               lines--;

               fxLeftEdge += fdxLeftEdge;
               fxRightEdge += fdxRightEdge;


               fError += fdError;
               if (fError >= 0) {
                  fError -= 0x00000800;
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowOuter);
                  fz += fdzOuter;
                  fogLeft += dfogOuter;
                  fr += fdrOuter;
                  fg += fdgOuter;
                  fb += fdbOuter;
                  fa += fdaOuter;
                  fs += fdsOuter;
                  ft += fdtOuter;
               }
               else {
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowInner);
                  fz += fdzInner;
                  fogLeft += dfogInner;
                  fr += fdrInner;
                  fg += fdgInner;
                  fb += fdbInner;
                  fa += fdaInner;
                  fs += fdsInner;
                  ft += fdtInner;
               }
            } /*while lines>0*/

         } /* for subTriangle */

      }
   }
}





struct persp_info
{
   GLenum filter;
   GLenum format;
   GLenum envmode;
   GLint smask, tmask;
   GLint twidth_log2;
   const GLchan *texture;
   GLfixed er, eg, eb, ea;   /* texture env color */
   GLint tbytesline, tsize;
};


static inline void
fast_persp_span(GLcontext *ctx, struct sw_span *span,
               struct persp_info *info)
{
   GLchan sample[4];  /* the filtered texture sample */

  /* Instead of defining a function for each mode, a test is done
   * between the outer and inner loops. This is to reduce code size
   * and complexity. Observe that an optimizing compiler kills
   * unused variables (for instance tf,sf,ti,si in case of GL_NEAREST).
   */


   GLuint i;
   GLfloat tex_coord[3], tex_step[3];
   GLchan *dest = span->array->rgba[0];

   tex_coord[0] = span->tex[0][0]  * (info->smask + 1);
   tex_step[0] = span->texStepX[0][0] * (info->smask + 1);
   tex_coord[1] = span->tex[0][1] * (info->tmask + 1);
   tex_step[1] = span->texStepX[0][1] * (info->tmask + 1);
   /* span->tex[0][2] only if 3D-texturing, here only 2D */
   tex_coord[2] = span->tex[0][3];
   tex_step[2] = span->texStepX[0][3];

   switch (info->filter) {
   case 0x2600:
      switch (info->format) {
      case 0x1907:
         switch (info->envmode) {
         case 0x2100:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLint s = ifloor(s_tmp) & info->smask; GLint t = ifloor(t_tmp) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; sample[0] = tex00[0]; sample[1] = tex00[1]; sample[2] = tex00[2]; sample[3] = 255;dest[0] = span->red * (sample[0] + 1u) >> (11 + 8); dest[1] = span->green * (sample[1] + 1u) >> (11 + 8); dest[2] = span->blue * (sample[2] + 1u) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1u) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x2101:
         case 0x1E01:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLint s = ifloor(s_tmp) & info->smask; GLint t = ifloor(t_tmp) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; sample[0] = tex00[0]; sample[1] = tex00[1]; sample[2] = tex00[2]; sample[3] = 255; dest[0] = sample[0]; dest[1] = sample[1]; dest[2] = sample[2]; dest[3] = ((span->alpha) >> 11);; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x0BE2:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLint s = ifloor(s_tmp) & info->smask; GLint t = ifloor(t_tmp) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; sample[0] = tex00[0]; sample[1] = tex00[1]; sample[2] = tex00[2]; sample[3] = 255;dest[0] = ((255 - sample[0]) * span->red + (sample[0] + 1) * info->er) >> (11 + 8); dest[1] = ((255 - sample[1]) * span->green + (sample[1] + 1) * info->eg) >> (11 + 8); dest[2] = ((255 - sample[2]) * span->blue + (sample[2] + 1) * info->eb) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x0104:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLint s = ifloor(s_tmp) & info->smask; GLint t = ifloor(t_tmp) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; sample[0] = tex00[0]; sample[1] = tex00[1]; sample[2] = tex00[2]; sample[3] = 255;{ GLint rSum = ((span->red) >> 11) + (GLint) sample[0]; GLint gSum = ((span->green) >> 11) + (GLint) sample[1]; GLint bSum = ((span->blue) >> 11) + (GLint) sample[2]; dest[0] = ( (rSum)<(255) ? (rSum) : (255) ); dest[1] = ( (gSum)<(255) ? (gSum) : (255) ); dest[2] = ( (bSum)<(255) ? (bSum) : (255) ); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         default:
            _mesa_problem(ctx, "bad tex env mode (5) in SPAN_LINEAR");
            return;
         }
         break;
      case 0x1908:
         switch(info->envmode) {
         case 0x2100:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLint s = ifloor(s_tmp) & info->smask; GLint t = ifloor(t_tmp) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; { (sample)[0] = (tex00)[0]; (sample)[1] = (tex00)[1]; (sample)[2] = (tex00)[2]; (sample)[3] = (tex00)[3]; };dest[0] = span->red * (sample[0] + 1u) >> (11 + 8); dest[1] = span->green * (sample[1] + 1u) >> (11 + 8); dest[2] = span->blue * (sample[2] + 1u) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1u) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x2101:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLint s = ifloor(s_tmp) & info->smask; GLint t = ifloor(t_tmp) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; { (sample)[0] = (tex00)[0]; (sample)[1] = (tex00)[1]; (sample)[2] = (tex00)[2]; (sample)[3] = (tex00)[3]; };dest[0] = ((255 - sample[3]) * span->red + ((sample[3] + 1) * sample[0] << 11)) >> (11 + 8); dest[1] = ((255 - sample[3]) * span->green + ((sample[3] + 1) * sample[1] << 11)) >> (11 + 8); dest[2] = ((255 - sample[3]) * span->blue + ((sample[3] + 1) * sample[2] << 11)) >> (11 + 8); dest[3] = ((span->alpha) >> 11); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x0BE2:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLint s = ifloor(s_tmp) & info->smask; GLint t = ifloor(t_tmp) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; { (sample)[0] = (tex00)[0]; (sample)[1] = (tex00)[1]; (sample)[2] = (tex00)[2]; (sample)[3] = (tex00)[3]; };dest[0] = ((255 - sample[0]) * span->red + (sample[0] + 1) * info->er) >> (11 + 8); dest[1] = ((255 - sample[1]) * span->green + (sample[1] + 1) * info->eg) >> (11 + 8); dest[2] = ((255 - sample[2]) * span->blue + (sample[2] + 1) * info->eb) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x0104:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLint s = ifloor(s_tmp) & info->smask; GLint t = ifloor(t_tmp) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; { (sample)[0] = (tex00)[0]; (sample)[1] = (tex00)[1]; (sample)[2] = (tex00)[2]; (sample)[3] = (tex00)[3]; };{ GLint rSum = ((span->red) >> 11) + (GLint) sample[0]; GLint gSum = ((span->green) >> 11) + (GLint) sample[1]; GLint bSum = ((span->blue) >> 11) + (GLint) sample[2]; dest[0] = ( (rSum)<(255) ? (rSum) : (255) ); dest[1] = ( (gSum)<(255) ? (gSum) : (255) ); dest[2] = ( (bSum)<(255) ? (bSum) : (255) ); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x1E01:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLint s = ifloor(s_tmp) & info->smask; GLint t = ifloor(t_tmp) & info->tmask; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; { (dest)[0] = (tex00)[0]; (dest)[1] = (tex00)[1]; (dest)[2] = (tex00)[2]; (dest)[3] = (tex00)[3]; }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         default:
            _mesa_problem(ctx, "bad tex env mode (6) in SPAN_LINEAR");
            return;
         }
         break;
      }
      break;

   case 0x2601:
      switch (info->format) {
      case 0x1907:
         switch (info->envmode) {
         case 0x2100:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLfixed s_fix = (((int) ((((s_tmp) * 2048.0f) >= 0.0F) ? (((s_tmp) * 2048.0f) + 0.5F) : (((s_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLfixed t_fix = (((int) ((((t_tmp) * 2048.0f) >= 0.0F) ? (((t_tmp) * 2048.0f) + 0.5F) : (((t_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLint s = ((((s_fix) & (~0x000007FF))) >> 11) & info->smask; GLint t = ((((t_fix) & (~0x000007FF))) >> 11) & info->tmask; GLfixed sf = s_fix & 0x000007FF; GLfixed tf = t_fix & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 3; const GLchan *tex11 = tex10 + 3; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = 255;dest[0] = span->red * (sample[0] + 1u) >> (11 + 8); dest[1] = span->green * (sample[1] + 1u) >> (11 + 8); dest[2] = span->blue * (sample[2] + 1u) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1u) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x2101:
         case 0x1E01:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLfixed s_fix = (((int) ((((s_tmp) * 2048.0f) >= 0.0F) ? (((s_tmp) * 2048.0f) + 0.5F) : (((s_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLfixed t_fix = (((int) ((((t_tmp) * 2048.0f) >= 0.0F) ? (((t_tmp) * 2048.0f) + 0.5F) : (((t_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLint s = ((((s_fix) & (~0x000007FF))) >> 11) & info->smask; GLint t = ((((t_fix) & (~0x000007FF))) >> 11) & info->tmask; GLfixed sf = s_fix & 0x000007FF; GLfixed tf = t_fix & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 3; const GLchan *tex11 = tex10 + 3; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = 255;{ (dest)[0] = (sample)[0]; (dest)[1] = (sample)[1]; (dest)[2] = (sample)[2]; (dest)[3] = (sample)[3]; }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x0BE2:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLfixed s_fix = (((int) ((((s_tmp) * 2048.0f) >= 0.0F) ? (((s_tmp) * 2048.0f) + 0.5F) : (((s_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLfixed t_fix = (((int) ((((t_tmp) * 2048.0f) >= 0.0F) ? (((t_tmp) * 2048.0f) + 0.5F) : (((t_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLint s = ((((s_fix) & (~0x000007FF))) >> 11) & info->smask; GLint t = ((((t_fix) & (~0x000007FF))) >> 11) & info->tmask; GLfixed sf = s_fix & 0x000007FF; GLfixed tf = t_fix & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 3; const GLchan *tex11 = tex10 + 3; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = 255;dest[0] = ((255 - sample[0]) * span->red + (sample[0] + 1) * info->er) >> (11 + 8); dest[1] = ((255 - sample[1]) * span->green + (sample[1] + 1) * info->eg) >> (11 + 8); dest[2] = ((255 - sample[2]) * span->blue + (sample[2] + 1) * info->eb) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x0104:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLfixed s_fix = (((int) ((((s_tmp) * 2048.0f) >= 0.0F) ? (((s_tmp) * 2048.0f) + 0.5F) : (((s_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLfixed t_fix = (((int) ((((t_tmp) * 2048.0f) >= 0.0F) ? (((t_tmp) * 2048.0f) + 0.5F) : (((t_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLint s = ((((s_fix) & (~0x000007FF))) >> 11) & info->smask; GLint t = ((((t_fix) & (~0x000007FF))) >> 11) & info->tmask; GLfixed sf = s_fix & 0x000007FF; GLfixed tf = t_fix & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 3 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 3; const GLchan *tex11 = tex10 + 3; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = 255;{ GLint rSum = ((span->red) >> 11) + (GLint) sample[0]; GLint gSum = ((span->green) >> 11) + (GLint) sample[1]; GLint bSum = ((span->blue) >> 11) + (GLint) sample[2]; dest[0] = ( (rSum)<(255) ? (rSum) : (255) ); dest[1] = ( (gSum)<(255) ? (gSum) : (255) ); dest[2] = ( (bSum)<(255) ? (bSum) : (255) ); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         default:
            _mesa_problem(ctx, "bad tex env mode (7) in SPAN_LINEAR");
            return;
         }
         break;
      case 0x1908:
         switch (info->envmode) {
         case 0x2100:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLfixed s_fix = (((int) ((((s_tmp) * 2048.0f) >= 0.0F) ? (((s_tmp) * 2048.0f) + 0.5F) : (((s_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLfixed t_fix = (((int) ((((t_tmp) * 2048.0f) >= 0.0F) ? (((t_tmp) * 2048.0f) + 0.5F) : (((t_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLint s = ((((s_fix) & (~0x000007FF))) >> 11) & info->smask; GLint t = ((((t_fix) & (~0x000007FF))) >> 11) & info->tmask; GLfixed sf = s_fix & 0x000007FF; GLfixed tf = t_fix & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 4; const GLchan *tex11 = tex10 + 4; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = (ti * (si * tex00[3] + sf * tex01[3]) + tf * (si * tex10[3] + sf * tex11[3])) >> 2 * 11;dest[0] = span->red * (sample[0] + 1u) >> (11 + 8); dest[1] = span->green * (sample[1] + 1u) >> (11 + 8); dest[2] = span->blue * (sample[2] + 1u) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1u) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x2101:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLfixed s_fix = (((int) ((((s_tmp) * 2048.0f) >= 0.0F) ? (((s_tmp) * 2048.0f) + 0.5F) : (((s_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLfixed t_fix = (((int) ((((t_tmp) * 2048.0f) >= 0.0F) ? (((t_tmp) * 2048.0f) + 0.5F) : (((t_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLint s = ((((s_fix) & (~0x000007FF))) >> 11) & info->smask; GLint t = ((((t_fix) & (~0x000007FF))) >> 11) & info->tmask; GLfixed sf = s_fix & 0x000007FF; GLfixed tf = t_fix & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 4; const GLchan *tex11 = tex10 + 4; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = (ti * (si * tex00[3] + sf * tex01[3]) + tf * (si * tex10[3] + sf * tex11[3])) >> 2 * 11;dest[0] = ((255 - sample[3]) * span->red + ((sample[3] + 1) * sample[0] << 11)) >> (11 + 8); dest[1] = ((255 - sample[3]) * span->green + ((sample[3] + 1) * sample[1] << 11)) >> (11 + 8); dest[2] = ((255 - sample[3]) * span->blue + ((sample[3] + 1) * sample[2] << 11)) >> (11 + 8); dest[3] = ((span->alpha) >> 11); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x0BE2:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLfixed s_fix = (((int) ((((s_tmp) * 2048.0f) >= 0.0F) ? (((s_tmp) * 2048.0f) + 0.5F) : (((s_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLfixed t_fix = (((int) ((((t_tmp) * 2048.0f) >= 0.0F) ? (((t_tmp) * 2048.0f) + 0.5F) : (((t_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLint s = ((((s_fix) & (~0x000007FF))) >> 11) & info->smask; GLint t = ((((t_fix) & (~0x000007FF))) >> 11) & info->tmask; GLfixed sf = s_fix & 0x000007FF; GLfixed tf = t_fix & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 4; const GLchan *tex11 = tex10 + 4; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = (ti * (si * tex00[3] + sf * tex01[3]) + tf * (si * tex10[3] + sf * tex11[3])) >> 2 * 11;dest[0] = ((255 - sample[0]) * span->red + (sample[0] + 1) * info->er) >> (11 + 8); dest[1] = ((255 - sample[1]) * span->green + (sample[1] + 1) * info->eg) >> (11 + 8); dest[2] = ((255 - sample[2]) * span->blue + (sample[2] + 1) * info->eb) >> (11 + 8); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x0104:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLfixed s_fix = (((int) ((((s_tmp) * 2048.0f) >= 0.0F) ? (((s_tmp) * 2048.0f) + 0.5F) : (((s_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLfixed t_fix = (((int) ((((t_tmp) * 2048.0f) >= 0.0F) ? (((t_tmp) * 2048.0f) + 0.5F) : (((t_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLint s = ((((s_fix) & (~0x000007FF))) >> 11) & info->smask; GLint t = ((((t_fix) & (~0x000007FF))) >> 11) & info->tmask; GLfixed sf = s_fix & 0x000007FF; GLfixed tf = t_fix & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 4; const GLchan *tex11 = tex10 + 4; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = (ti * (si * tex00[3] + sf * tex01[3]) + tf * (si * tex10[3] + sf * tex11[3])) >> 2 * 11;{ GLint rSum = ((span->red) >> 11) + (GLint) sample[0]; GLint gSum = ((span->green) >> 11) + (GLint) sample[1]; GLint bSum = ((span->blue) >> 11) + (GLint) sample[2]; dest[0] = ( (rSum)<(255) ? (rSum) : (255) ); dest[1] = ( (gSum)<(255) ? (gSum) : (255) ); dest[2] = ( (bSum)<(255) ? (bSum) : (255) ); dest[3] = span->alpha * (sample[3] + 1) >> (11 + 8); }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         case 0x1E01:
            for (i = 0; i < span->end; i++) { GLdouble invQ = tex_coord[2] ? (1.0 / tex_coord[2]) : 1.0; GLfloat s_tmp = (GLfloat) (tex_coord[0] * invQ); GLfloat t_tmp = (GLfloat) (tex_coord[1] * invQ); GLfixed s_fix = (((int) ((((s_tmp) * 2048.0f) >= 0.0F) ? (((s_tmp) * 2048.0f) + 0.5F) : (((s_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLfixed t_fix = (((int) ((((t_tmp) * 2048.0f) >= 0.0F) ? (((t_tmp) * 2048.0f) + 0.5F) : (((t_tmp) * 2048.0f) - 0.5F)))) - 0x00000400; GLint s = ((((s_fix) & (~0x000007FF))) >> 11) & info->smask; GLint t = ((((t_fix) & (~0x000007FF))) >> 11) & info->tmask; GLfixed sf = s_fix & 0x000007FF; GLfixed tf = t_fix & 0x000007FF; GLfixed si = 0x000007FF - sf; GLfixed ti = 0x000007FF - tf; GLint pos = (t << info->twidth_log2) + s; const GLchan *tex00 = info->texture + 4 * pos; const GLchan *tex10 = tex00 + info->tbytesline; const GLchan *tex01 = tex00 + 4; const GLchan *tex11 = tex10 + 4; (void) ti; (void) si; if (t == info->tmask) { tex10 -= info->tsize; tex11 -= info->tsize; } if (s == info->smask) { tex01 -= info->tbytesline; tex11 -= info->tbytesline; } sample[0] = (ti * (si * tex00[0] + sf * tex01[0]) + tf * (si * tex10[0] + sf * tex11[0])) >> 2 * 11; sample[1] = (ti * (si * tex00[1] + sf * tex01[1]) + tf * (si * tex10[1] + sf * tex11[1])) >> 2 * 11; sample[2] = (ti * (si * tex00[2] + sf * tex01[2]) + tf * (si * tex10[2] + sf * tex11[2])) >> 2 * 11; sample[3] = (ti * (si * tex00[3] + sf * tex01[3]) + tf * (si * tex10[3] + sf * tex11[3])) >> 2 * 11;{ (dest)[0] = (sample)[0]; (dest)[1] = (sample)[1]; (dest)[2] = (sample)[2]; (dest)[3] = (sample)[3]; }; span->red += span->redStep; span->green += span->greenStep; span->blue += span->blueStep; span->alpha += span->alphaStep; tex_coord[0] += tex_step[0]; tex_coord[1] += tex_step[1]; tex_coord[2] += tex_step[2]; dest += 4; };
            break;
         default:
            _mesa_problem(ctx, "bad tex env mode (8) in SPAN_LINEAR");
            return;
         }
         break;
      }
      break;
   }

   ;
   _mesa_write_rgba_span(ctx, span);

}


/*
 * Render an perspective corrected RGB/RGBA textured triangle.
 * The Q (aka V in Mesa) coordinate must be zero such that the divide
 * by interpolated Q/W comes out right.
 *
 */
/*
 * Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom triangle rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated across the triangle:
 *    INTERP_Z        - if defined, interpolate Z values
 *    INTERP_FOG      - if defined, interpolate fog values
 *    INTERP_RGB      - if defined, interpolate RGB values
 *    INTERP_ALPHA    - if defined, interpolate Alpha values (req's INTERP_RGB)
 *    INTERP_SPEC     - if defined, interpolate specular RGB values
 *    INTERP_INDEX    - if defined, interpolate color index values
 *    INTERP_INT_TEX  - if defined, interpolate integer ST texcoords
 *                         (fast, simple 2-D texture mapping)
 *    INTERP_TEX      - if defined, interpolate set 0 float STRQ texcoords
 *                         NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_MULTITEX - if defined, interpolate N units of STRQ texcoords
 *    INTERP_FLOAT_RGBA - if defined, interpolate RGBA with floating point
 *    INTERP_FLOAT_SPEC - if defined, interpolate specular with floating point
 *
 * When one can directly address pixels in the color buffer the following
 * macros can be defined and used to compute pixel addresses during
 * rasterization (see pRow):
 *    PIXEL_TYPE          - the datatype of a pixel (GLubyte, GLushort, GLuint)
 *    BYTES_PER_ROW       - number of bytes per row in the color buffer
 *    PIXEL_ADDRESS(X,Y)  - returns the address of pixel at (X,Y) where
 *                          Y==0 at bottom of screen and increases upward.
 *
 * Similarly, for direct depth buffer access, this type is used for depth
 * buffer addressing:
 *    DEPTH_TYPE          - either GLushort or GLuint
 *
 * Optionally, one may provide one-time setup code per triangle:
 *    SETUP_CODE    - code which is to be executed once per triangle
 *    CLEANUP_CODE    - code to execute at end of triangle
 *
 * The following macro MUST be defined:
 *    RENDER_SPAN(span) - code to write a span of pixels.
 *
 * This code was designed for the origin to be in the lower-left corner.
 *
 * Inspired by triangle rasterizer code written by Allen Akin.  Thanks Allen!
 */


/*
 * This is a bit of a hack, but it's a centralized place to enable floating-
 * point color interpolation when GLchan is actually floating point.
 */



static void persp_textured_triangle(GLcontext *ctx, const SWvertex *v0,
                                 const SWvertex *v1,
                                 const SWvertex *v2 )
{
   typedef struct {
        const SWvertex *v0, *v1;   /* Y(v0) < Y(v1) */
        GLfloat dx;    /* X(v1) - X(v0) */
        GLfloat dy;    /* Y(v1) - Y(v0) */
        GLfixed fdxdy; /* dx/dy in fixed-point */
        GLfixed fsx;   /* first sample point x coord */
        GLfixed fsy;
        GLfloat adjy;  /* adjust from v[0]->fy to fsy, scaled */
        GLint lines;   /* number of lines to be sampled on this edge */
        GLfixed fx0;   /* fixed pt X of lower endpoint */
   } EdgeT;

   const GLint depthBits = ctx->Visual.depthBits;
   const GLfloat maxDepth = ctx->DepthMaxF;
   EdgeT eMaj, eTop, eBot;
   GLfloat oneOverArea;
   const SWvertex *vMin, *vMid, *vMax;  /* Y(vMin)<=Y(vMid)<=Y(vMax) */
   float bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;
   const GLint snapMask = ~((0x00000800 / (1 << 4)) - 1); /* for x/y coord snapping */
   GLfixed vMin_fx, vMin_fy, vMid_fx, vMid_fy, vMax_fx, vMax_fy;

   struct sw_span span;

    (span).primitive = (0x0009);
    (span).interpMask = (0);
    (span).arrayMask = (0);
    (span).start = 0;
    (span).end = (0);
    (span).facing = 0;
    (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays;

   /*
   printf("%s()\n", __FUNCTION__);
   printf("  %g, %g, %g\n", v0->win[0], v0->win[1], v0->win[2]);
   printf("  %g, %g, %g\n", v1->win[0], v1->win[1], v1->win[2]);
   printf("  %g, %g, %g\n", v2->win[0], v2->win[1], v2->win[2]);
    */

   /* Compute fixed point x,y coords w/ half-pixel offsets and snapping.
    * And find the order of the 3 vertices along the Y axis.
    */
   {
      const GLfixed fy0 = (((int) ((((v0->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v0->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v0->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy1 = (((int) ((((v1->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v1->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v1->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy2 = (((int) ((((v2->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v2->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v2->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;

      if (fy0 <= fy1) {
         if (fy1 <= fy2) {
            /* y0 <= y1 <= y2 */
            vMin = v0;   vMid = v1;   vMax = v2;
            vMin_fy = fy0;  vMid_fy = fy1;  vMax_fy = fy2;
         }
         else if (fy2 <= fy0) {
            /* y2 <= y0 <= y1 */
            vMin = v2;   vMid = v0;   vMax = v1;
            vMin_fy = fy2;  vMid_fy = fy0;  vMax_fy = fy1;
         }
         else {
            /* y0 <= y2 <= y1 */
            vMin = v0;   vMid = v2;   vMax = v1;
            vMin_fy = fy0;  vMid_fy = fy2;  vMax_fy = fy1;
            bf = -bf;
         }
      }
      else {
         if (fy0 <= fy2) {
            /* y1 <= y0 <= y2 */
            vMin = v1;   vMid = v0;   vMax = v2;
            vMin_fy = fy1;  vMid_fy = fy0;  vMax_fy = fy2;
            bf = -bf;
         }
         else if (fy2 <= fy1) {
            /* y2 <= y1 <= y0 */
            vMin = v2;   vMid = v1;   vMax = v0;
            vMin_fy = fy2;  vMid_fy = fy1;  vMax_fy = fy0;
            bf = -bf;
         }
         else {
            /* y1 <= y2 <= y0 */
            vMin = v1;   vMid = v2;   vMax = v0;
            vMin_fy = fy1;  vMid_fy = fy2;  vMax_fy = fy0;
         }
      }

      /* fixed point X coords */
      vMin_fx = (((int) ((((vMin->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMin->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMin->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMid_fx = (((int) ((((vMid->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMid->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMid->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMax_fx = (((int) ((((vMax->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMax->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMax->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
   }

   /* vertex/edge relationship */
   eMaj.v0 = vMin;   eMaj.v1 = vMax;   /*TODO: .v1's not needed */
   eTop.v0 = vMid;   eTop.v1 = vMax;
   eBot.v0 = vMin;   eBot.v1 = vMid;

   /* compute deltas for each edge:  vertex[upper] - vertex[lower] */
   eMaj.dx = ((vMax_fx - vMin_fx) * (1.0F / 2048.0f));
   eMaj.dy = ((vMax_fy - vMin_fy) * (1.0F / 2048.0f));
   eTop.dx = ((vMax_fx - vMid_fx) * (1.0F / 2048.0f));
   eTop.dy = ((vMax_fy - vMid_fy) * (1.0F / 2048.0f));
   eBot.dx = ((vMid_fx - vMin_fx) * (1.0F / 2048.0f));
   eBot.dy = ((vMid_fy - vMin_fy) * (1.0F / 2048.0f));

   /* compute area, oneOverArea and perform backface culling */
   {
      const GLfloat area = eMaj.dx * eBot.dy - eBot.dx * eMaj.dy;

      /* Do backface culling */
      if (area * bf < 0.0)
         return;

      if (IS_INF_OR_NAN(area) || area == 0.0F)
         return;

      oneOverArea = 1.0F / area;
   }

   ctx->OcclusionResult = 0x1;
   span.facing = ctx->_Facing; /* for 2-sided stencil test */

   /* Edge setup.  For a triangle strip these could be reused... */
   {
      eMaj.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eMaj.lines = (((((vMax_fy - eMaj.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eMaj.lines > 0) {
         GLfloat dxdy = eMaj.dx / eMaj.dy;
         eMaj.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eMaj.adjy = (GLfloat) (eMaj.fsy - vMin_fy);  /* SCALED! */
         eMaj.fx0 = vMin_fx;
         eMaj.fsx = eMaj.fx0 + (GLfixed) (eMaj.adjy * dxdy);
      }
      else {
         return;  /*CULLED*/
      }

      eTop.fsy = (((vMid_fy) + 0x00000800 - 1) & (~0x000007FF));
      eTop.lines = (((((vMax_fy - eTop.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eTop.lines > 0) {
         GLfloat dxdy = eTop.dx / eTop.dy;
         eTop.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eTop.adjy = (GLfloat) (eTop.fsy - vMid_fy); /* SCALED! */
         eTop.fx0 = vMid_fx;
         eTop.fsx = eTop.fx0 + (GLfixed) (eTop.adjy * dxdy);
      }

      eBot.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eBot.lines = (((((vMid_fy - eBot.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eBot.lines > 0) {
         GLfloat dxdy = eBot.dx / eBot.dy;
         eBot.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eBot.adjy = (GLfloat) (eBot.fsy - vMin_fy);  /* SCALED! */
         eBot.fx0 = vMin_fx;
         eBot.fsx = eBot.fx0 + (GLfixed) (eBot.adjy * dxdy);
      }
   }

   /*
    * Conceptually, we view a triangle as two subtriangles
    * separated by a perfectly horizontal line.  The edge that is
    * intersected by this line is one with maximal absolute dy; we
    * call it a ``major'' edge.  The other two edges are the
    * ``top'' edge (for the upper subtriangle) and the ``bottom''
    * edge (for the lower subtriangle).  If either of these two
    * edges is horizontal or very close to horizontal, the
    * corresponding subtriangle might cover zero sample points;
    * we take care to handle such cases, for performance as well
    * as correctness.
    *
    * By stepping rasterization parameters along the major edge,
    * we can avoid recomputing them at the discontinuity where
    * the top and bottom edges meet.  However, this forces us to
    * be able to scan both left-to-right and right-to-left.
    * Also, we must determine whether the major edge is at the
    * left or right side of the triangle.  We do this by
    * computing the magnitude of the cross-product of the major
    * and top edges.  Since this magnitude depends on the sine of
    * the angle between the two edges, its sign tells us whether
    * we turn to the left or to the right when travelling along
    * the major edge to the top edge, and from this we infer
    * whether the major edge is on the left or the right.
    *
    * Serendipitously, this cross-product magnitude is also a
    * value we need to compute the iteration parameter
    * derivatives for the triangle, and it can be used to perform
    * backface culling because its sign tells us whether the
    * triangle is clockwise or counterclockwise.  In this code we
    * refer to it as ``area'' because it's also proportional to
    * the pixel area of the triangle.
    */

   {
      GLint scan_from_left_to_right;  /* true if scanning left-to-right */
      GLfloat dzdx, dzdy;
      GLfloat dfogdy;
      GLfloat drdx, drdy;
      GLfloat dgdx, dgdy;
      GLfloat dbdx, dbdy;
      GLfloat dadx, dady;
      GLfloat dsdx, dsdy;
      GLfloat dtdx, dtdy;
      GLfloat dudx, dudy;
      GLfloat dvdx, dvdy;

      /*
       * Execute user-supplied setup code
       */
      struct persp_info info; const struct gl_texture_unit *unit = ctx->Texture.Unit+0; const struct gl_texture_object *obj = unit->Current2D; const GLint b = obj->BaseLevel; info.texture = (const GLchan *) obj->Image[b]->Data; info.twidth_log2 = obj->Image[b]->WidthLog2; info.smask = obj->Image[b]->Width - 1; info.tmask = obj->Image[b]->Height - 1; info.format = obj->Image[b]->Format; info.filter = obj->MinFilter; info.envmode = unit->EnvMode; if (info.envmode == 0x0BE2) { info.er = (((int) ((((unit->EnvColor[0] * 255.0F) * 2048.0f) >= 0.0F) ? (((unit->EnvColor[0] * 255.0F) * 2048.0f) + 0.5F) : (((unit->EnvColor[0] * 255.0F) * 2048.0f) - 0.5F)))); info.eg = (((int) ((((unit->EnvColor[1] * 255.0F) * 2048.0f) >= 0.0F) ? (((unit->EnvColor[1] * 255.0F) * 2048.0f) + 0.5F) : (((unit->EnvColor[1] * 255.0F) * 2048.0f) - 0.5F)))); info.eb = (((int) ((((unit->EnvColor[2] * 255.0F) * 2048.0f) >= 0.0F) ? (((unit->EnvColor[2] * 255.0F) * 2048.0f) + 0.5F) : (((unit->EnvColor[2] * 255.0F) * 2048.0f) - 0.5F)))); info.ea = (((int) ((((unit->EnvColor[3] * 255.0F) * 2048.0f) >= 0.0F) ? (((unit->EnvColor[3] * 255.0F) * 2048.0f) + 0.5F) : (((unit->EnvColor[3] * 255.0F) * 2048.0f) - 0.5F)))); } if (!info.texture) { return; } switch (info.format) { case 0x1906: case 0x1909: case 0x8049: info.tbytesline = obj->Image[b]->Width; break; case 0x190A: info.tbytesline = obj->Image[b]->Width * 2; break; case 0x1907: info.tbytesline = obj->Image[b]->Width * 3; break; case 0x1908: info.tbytesline = obj->Image[b]->Width * 4; break; default: _mesa_problem(0, "Bad texture format in persp_textured_triangle"); return; } info.tsize = obj->Image[b]->Height * info.tbytesline;

      scan_from_left_to_right = (oneOverArea < 0.0F);


      /* compute d?/dx and d?/dy derivatives */
      span.interpMask |= 0x008;
      {
         GLfloat eMaj_dz, eBot_dz;
         eMaj_dz = vMax->win[2] - vMin->win[2];
         eBot_dz = vMid->win[2] - vMin->win[2];
         dzdx = oneOverArea * (eMaj_dz * eBot.dy - eMaj.dy * eBot_dz);
         if (dzdx > maxDepth || dzdx < -maxDepth) {
            /* probably a sliver triangle */
            dzdx = 0.0;
            dzdy = 0.0;
         }
         else {
            dzdy = oneOverArea * (eMaj.dx * eBot_dz - eMaj_dz * eBot.dx);
         }
         if (depthBits <= 16)
            span.zStep = (((int) ((((dzdx) * 2048.0f) >= 0.0F) ? (((dzdx) * 2048.0f) + 0.5F) : (((dzdx) * 2048.0f) - 0.5F))));
         else
            span.zStep = (GLint) dzdx;
      }
      span.interpMask |= 0x010;
      {
         const GLfloat eMaj_dfog = vMax->fog - vMin->fog;
         const GLfloat eBot_dfog = vMid->fog - vMin->fog;
         span.fogStep = oneOverArea * (eMaj_dfog * eBot.dy - eMaj.dy * eBot_dfog);
         dfogdy = oneOverArea * (eMaj.dx * eBot_dfog - eMaj_dfog * eBot.dx);
      }
      span.interpMask |= 0x001;
      if (ctx->Light.ShadeModel == 0x1D01) {
         GLfloat eMaj_dr, eBot_dr;
         GLfloat eMaj_dg, eBot_dg;
         GLfloat eMaj_db, eBot_db;
         GLfloat eMaj_da, eBot_da;
         eMaj_dr = (GLfloat) ((GLint) vMax->color[0] -
                             (GLint) vMin->color[0]);
         eBot_dr = (GLfloat) ((GLint) vMid->color[0] -
                             (GLint) vMin->color[0]);
         drdx = oneOverArea * (eMaj_dr * eBot.dy - eMaj.dy * eBot_dr);
         span.redStep = (((int) ((((drdx) * 2048.0f) >= 0.0F) ? (((drdx) * 2048.0f) + 0.5F) : (((drdx) * 2048.0f) - 0.5F))));
         drdy = oneOverArea * (eMaj.dx * eBot_dr - eMaj_dr * eBot.dx);
         eMaj_dg = (GLfloat) ((GLint) vMax->color[1] -
                             (GLint) vMin->color[1]);
         eBot_dg = (GLfloat) ((GLint) vMid->color[1] -
                             (GLint) vMin->color[1]);
         dgdx = oneOverArea * (eMaj_dg * eBot.dy - eMaj.dy * eBot_dg);
         span.greenStep = (((int) ((((dgdx) * 2048.0f) >= 0.0F) ? (((dgdx) * 2048.0f) + 0.5F) : (((dgdx) * 2048.0f) - 0.5F))));
         dgdy = oneOverArea * (eMaj.dx * eBot_dg - eMaj_dg * eBot.dx);
         eMaj_db = (GLfloat) ((GLint) vMax->color[2] -
                             (GLint) vMin->color[2]);
         eBot_db = (GLfloat) ((GLint) vMid->color[2] -
                             (GLint) vMin->color[2]);
         dbdx = oneOverArea * (eMaj_db * eBot.dy - eMaj.dy * eBot_db);
         span.blueStep = (((int) ((((dbdx) * 2048.0f) >= 0.0F) ? (((dbdx) * 2048.0f) + 0.5F) : (((dbdx) * 2048.0f) - 0.5F))));
         dbdy = oneOverArea * (eMaj.dx * eBot_db - eMaj_db * eBot.dx);
         eMaj_da = (GLfloat) ((GLint) vMax->color[3] -
                             (GLint) vMin->color[3]);
         eBot_da = (GLfloat) ((GLint) vMid->color[3] -
                             (GLint) vMin->color[3]);
         dadx = oneOverArea * (eMaj_da * eBot.dy - eMaj.dy * eBot_da);
         span.alphaStep = (((int) ((((dadx) * 2048.0f) >= 0.0F) ? (((dadx) * 2048.0f) + 0.5F) : (((dadx) * 2048.0f) - 0.5F))));
         dady = oneOverArea * (eMaj.dx * eBot_da - eMaj_da * eBot.dx);
      }
      else {
         ;
         span.interpMask |= 0x200;
         drdx = drdy = 0.0F;
         dgdx = dgdy = 0.0F;
         dbdx = dbdy = 0.0F;
         span.redStep = 0;
         span.greenStep = 0;
         span.blueStep = 0;
         dadx = dady = 0.0F;
         span.alphaStep = 0;
      }
      span.interpMask |= 0x020;
      {
         GLfloat wMax = vMax->win[3];
         GLfloat wMin = vMin->win[3];
         GLfloat wMid = vMid->win[3];
         GLfloat eMaj_ds, eBot_ds;
         GLfloat eMaj_dt, eBot_dt;
         GLfloat eMaj_du, eBot_du;
         GLfloat eMaj_dv, eBot_dv;

         eMaj_ds = vMax->texcoord[0][0] * wMax - vMin->texcoord[0][0] * wMin;
         eBot_ds = vMid->texcoord[0][0] * wMid - vMin->texcoord[0][0] * wMin;
         dsdx = oneOverArea * (eMaj_ds * eBot.dy - eMaj.dy * eBot_ds);
         dsdy = oneOverArea * (eMaj.dx * eBot_ds - eMaj_ds * eBot.dx);
         span.texStepX[0][0] = dsdx;
         span.texStepY[0][0] = dsdy;

         eMaj_dt = vMax->texcoord[0][1] * wMax - vMin->texcoord[0][1] * wMin;
         eBot_dt = vMid->texcoord[0][1] * wMid - vMin->texcoord[0][1] * wMin;
         dtdx = oneOverArea * (eMaj_dt * eBot.dy - eMaj.dy * eBot_dt);
         dtdy = oneOverArea * (eMaj.dx * eBot_dt - eMaj_dt * eBot.dx);
         span.texStepX[0][1] = dtdx;
         span.texStepY[0][1] = dtdy;

         eMaj_du = vMax->texcoord[0][2] * wMax - vMin->texcoord[0][2] * wMin;
         eBot_du = vMid->texcoord[0][2] * wMid - vMin->texcoord[0][2] * wMin;
         dudx = oneOverArea * (eMaj_du * eBot.dy - eMaj.dy * eBot_du);
         dudy = oneOverArea * (eMaj.dx * eBot_du - eMaj_du * eBot.dx);
         span.texStepX[0][2] = dudx;
         span.texStepY[0][2] = dudy;

         eMaj_dv = vMax->texcoord[0][3] * wMax - vMin->texcoord[0][3] * wMin;
         eBot_dv = vMid->texcoord[0][3] * wMid - vMin->texcoord[0][3] * wMin;
         dvdx = oneOverArea * (eMaj_dv * eBot.dy - eMaj.dy * eBot_dv);
         dvdy = oneOverArea * (eMaj.dx * eBot_dv - eMaj_dv * eBot.dx);
         span.texStepX[0][3] = dvdx;
         span.texStepY[0][3] = dvdy;
      }

      /*
       * We always sample at pixel centers.  However, we avoid
       * explicit half-pixel offsets in this code by incorporating
       * the proper offset in each of x and y during the
       * transformation to window coordinates.
       *
       * We also apply the usual rasterization rules to prevent
       * cracks and overlaps.  A pixel is considered inside a
       * subtriangle if it meets all of four conditions: it is on or
       * to the right of the left edge, strictly to the left of the
       * right edge, on or below the top edge, and strictly above
       * the bottom edge.  (Some edges may be degenerate.)
       *
       * The following discussion assumes left-to-right scanning
       * (that is, the major edge is on the left); the right-to-left
       * case is a straightforward variation.
       *
       * We start by finding the half-integral y coordinate that is
       * at or below the top of the triangle.  This gives us the
       * first scan line that could possibly contain pixels that are
       * inside the triangle.
       *
       * Next we creep down the major edge until we reach that y,
       * and compute the corresponding x coordinate on the edge.
       * Then we find the half-integral x that lies on or just
       * inside the edge.  This is the first pixel that might lie in
       * the interior of the triangle.  (We won't know for sure
       * until we check the other edges.)
       *
       * As we rasterize the triangle, we'll step down the major
       * edge.  For each step in y, we'll move an integer number
       * of steps in x.  There are two possible x step sizes, which
       * we'll call the ``inner'' step (guaranteed to land on the
       * edge or inside it) and the ``outer'' step (guaranteed to
       * land on the edge or outside it).  The inner and outer steps
       * differ by one.  During rasterization we maintain an error
       * term that indicates our distance from the true edge, and
       * select either the inner step or the outer step, whichever
       * gets us to the first pixel that falls inside the triangle.
       *
       * All parameters (z, red, etc.) as well as the buffer
       * addresses for color and z have inner and outer step values,
       * so that we can increment them appropriately.  This method
       * eliminates the need to adjust parameters by creeping a
       * sub-pixel amount into the triangle at each scanline.
       */

      {
         int subTriangle;
         GLfixed fx;
         GLfixed fxLeftEdge = 0, fxRightEdge = 0;
         GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
         GLfixed fdxOuter;
         int idxOuter;
         float dxOuter;
         GLfixed fError = 0, fdError = 0;
         float adjx, adjy;
         GLfixed fy;
         GLushort *zRow = 0;
         int dZRowOuter = 0, dZRowInner;  /* offset in bytes */
         GLfixed fz = 0, fdzOuter = 0, fdzInner;
         GLfloat fogLeft = 0, dfogOuter = 0, dfogInner;
         GLfixed fr = 0, fdrOuter = 0, fdrInner;
         GLfixed fg = 0, fdgOuter = 0, fdgInner;
         GLfixed fb = 0, fdbOuter = 0, fdbInner;
         GLfixed fa = 0, fdaOuter = 0, fdaInner;
         GLfloat sLeft=0, dsOuter=0, dsInner;
         GLfloat tLeft=0, dtOuter=0, dtInner;
         GLfloat uLeft=0, duOuter=0, duInner;
         GLfloat vLeft=0, dvOuter=0, dvInner;

         for (subTriangle=0; subTriangle<=1; subTriangle++) {
            EdgeT *eLeft, *eRight;
            int setupLeft, setupRight;
            int lines;

            if (subTriangle==0) {
               /* bottom half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eBot;
                  lines = eRight->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
               else {
                  eLeft = &eBot;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
            }
            else {
               /* top half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eTop;
                  lines = eRight->lines;
                  setupLeft = 0;
                  setupRight = 1;
               }
               else {
                  eLeft = &eTop;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 0;
               }
               if (lines == 0)
                  return;
            }

            if (setupLeft && eLeft->lines > 0) {
               const SWvertex *vLower;
               GLfixed fsx = eLeft->fsx;
               fx = (((fsx) + 0x00000800 - 1) & (~0x000007FF));
               fError = fx - fsx - 0x00000800;
               fxLeftEdge = fsx - 1;
               fdxLeftEdge = eLeft->fdxdy;
               fdxOuter = ((fdxLeftEdge - 1) & (~0x000007FF));
               fdError = fdxOuter - fdxLeftEdge + 0x00000800;
               idxOuter = ((fdxOuter) >> 11);
               dxOuter = (float) idxOuter;
               (void) dxOuter;

               fy = eLeft->fsy;
               span.y = ((fy) >> 11);

               adjx = (float)(fx - eLeft->fx0);  /* SCALED! */
               adjy = eLeft->adjy;              /* SCALED! */
               vLower = eLeft->v0;

               /*
                * Now we need the set of parameter (z, color, etc.) values at
                * the point (fx, fy).  This gives us properly-sampled parameter
                * values that we can step from pixel to pixel.  Furthermore,
                * although we might have intermediate results that overflow
                * the normal parameter range when we step temporarily outside
                * the triangle, we shouldn't overflow or underflow for any
                * pixel that's actually inside the triangle.
                */

               {
                  GLfloat z0 = vLower->win[2];
                  if (depthBits <= 16) {
                     /* interpolate fixed-pt values */
                     GLfloat tmp = (z0 * 2048.0f +
                                    dzdx * adjx + dzdy * adjy) + 0x00000400;
                     if (tmp < 0xffffffff / 2)
                        fz = (GLfixed) tmp;
                     else
                        fz = 0xffffffff / 2;
                     fdzOuter = (((int) ((((dzdy + dxOuter * dzdx) * 2048.0f) >= 0.0F) ? (((dzdy + dxOuter * dzdx) * 2048.0f) + 0.5F) : (((dzdy + dxOuter * dzdx) * 2048.0f) - 0.5F))));
                  }
                  else {
                     /* interpolate depth values exactly */
                     fz = (GLint) (z0 + dzdx * ((adjx) * (1.0F / 2048.0f))
                                   + dzdy * ((adjy) * (1.0F / 2048.0f)));
                     fdzOuter = (GLint) (dzdy + dxOuter * dzdx);
                  }
                  zRow = (GLushort *)
                    _mesa_zbuffer_address(ctx, ((fxLeftEdge) >> 11), span.y);
                  dZRowOuter = (ctx->DrawBuffer->Width + idxOuter) * sizeof(GLushort);
               }
               fogLeft = vLower->fog + (span.fogStep * adjx + dfogdy * adjy)
                                       * (1.0F/2048.0f);
               dfogOuter = dfogdy + dxOuter * span.fogStep;
               if (ctx->Light.ShadeModel == 0x1D01) {
                  fr = (GLfixed) (((vLower->color[0]) << 11)
                                   + drdx * adjx + drdy * adjy) + 0x00000400;
                  fdrOuter = (((int) ((((drdy + dxOuter * drdx) * 2048.0f) >= 0.0F) ? (((drdy + dxOuter * drdx) * 2048.0f) + 0.5F) : (((drdy + dxOuter * drdx) * 2048.0f) - 0.5F))));
                  fg = (GLfixed) (((vLower->color[1]) << 11)
                                   + dgdx * adjx + dgdy * adjy) + 0x00000400;
                  fdgOuter = (((int) ((((dgdy + dxOuter * dgdx) * 2048.0f) >= 0.0F) ? (((dgdy + dxOuter * dgdx) * 2048.0f) + 0.5F) : (((dgdy + dxOuter * dgdx) * 2048.0f) - 0.5F))));
                  fb = (GLfixed) (((vLower->color[2]) << 11)
                                    + dbdx * adjx + dbdy * adjy) + 0x00000400;
                  fdbOuter = (((int) ((((dbdy + dxOuter * dbdx) * 2048.0f) >= 0.0F) ? (((dbdy + dxOuter * dbdx) * 2048.0f) + 0.5F) : (((dbdy + dxOuter * dbdx) * 2048.0f) - 0.5F))));
                  fa = (GLfixed) (((vLower->color[3]) << 11)
                                   + dadx * adjx + dady * adjy) + 0x00000400;
                  fdaOuter = (((int) ((((dady + dxOuter * dadx) * 2048.0f) >= 0.0F) ? (((dady + dxOuter * dadx) * 2048.0f) + 0.5F) : (((dady + dxOuter * dadx) * 2048.0f) - 0.5F))));
               }
               else {
                  ;
                  fr = ((v2->color[0]) << 11);
                  fg = ((v2->color[1]) << 11);
                  fb = ((v2->color[2]) << 11);
                  fdrOuter = fdgOuter = fdbOuter = 0;
                  fa =  ((v2->color[3]) << 11);
                  fdaOuter = 0;
               }
               {
                  GLfloat invW = vLower->win[3];
                  GLfloat s0, t0, u0, v0;
                  s0 = vLower->texcoord[0][0] * invW;
                  sLeft = s0 + (span.texStepX[0][0] * adjx + dsdy * adjy)
                     * (1.0F/2048.0f);
                  dsOuter = dsdy + dxOuter * span.texStepX[0][0];
                  t0 = vLower->texcoord[0][1] * invW;
                  tLeft = t0 + (span.texStepX[0][1] * adjx + dtdy * adjy)
                     * (1.0F/2048.0f);
                  dtOuter = dtdy + dxOuter * span.texStepX[0][1];
                  u0 = vLower->texcoord[0][2] * invW;
                  uLeft = u0 + (span.texStepX[0][2] * adjx + dudy * adjy)
                     * (1.0F/2048.0f);
                  duOuter = dudy + dxOuter * span.texStepX[0][2];
                  v0 = vLower->texcoord[0][3] * invW;
                  vLeft = v0 + (span.texStepX[0][3] * adjx + dvdy * adjy)
                     * (1.0F/2048.0f);
                  dvOuter = dvdy + dxOuter * span.texStepX[0][3];
               }

            } /*if setupLeft*/


            if (setupRight && eRight->lines>0) {
               fxRightEdge = eRight->fsx - 1;
               fdxRightEdge = eRight->fdxdy;
            }

            if (lines==0) {
               continue;
            }


            /* Rasterize setup */
            dZRowInner = dZRowOuter + sizeof(GLushort);
            fdzInner = fdzOuter + span.zStep;
            dfogInner = dfogOuter + span.fogStep;
            fdrInner = fdrOuter + span.redStep;
            fdgInner = fdgOuter + span.greenStep;
            fdbInner = fdbOuter + span.blueStep;
            fdaInner = fdaOuter + span.alphaStep;
            dsInner = dsOuter + span.texStepX[0][0];
            dtInner = dtOuter + span.texStepX[0][1];
            duInner = duOuter + span.texStepX[0][2];
            dvInner = dvOuter + span.texStepX[0][3];

            while (lines > 0) {
               /* initialize the span interpolants to the leftmost value */
               /* ff = fixed-pt fragment */
               const GLint right = ((fxRightEdge) >> 11);

               span.x = ((fxLeftEdge) >> 11);

               if (right <= span.x)
                  span.end = 0;
               else
                  span.end = right - span.x;

               span.z = fz;
               span.fog = fogLeft;
               span.red = fr;
               span.green = fg;
               span.blue = fb;
               span.alpha = fa;

               span.tex[0][0] = sLeft;
               span.tex[0][1] = tLeft;
               span.tex[0][2] = uLeft;
               span.tex[0][3] = vLeft;


               {
                  /* need this to accomodate round-off errors */
                  const GLint len = right - span.x - 1;
                  GLfixed ffrend = span.red + len * span.redStep;
                  GLfixed ffgend = span.green + len * span.greenStep;
                  GLfixed ffbend = span.blue + len * span.blueStep;
                  if (ffrend < 0) {
                     span.red -= ffrend;
                     if (span.red < 0)
                        span.red = 0;
                  }
                  if (ffgend < 0) {
                     span.green -= ffgend;
                     if (span.green < 0)
                        span.green = 0;
                  }
                  if (ffbend < 0) {
                     span.blue -= ffbend;
                     if (span.blue < 0)
                        span.blue = 0;
                  }
               }
               {
                  const GLint len = right - span.x - 1;
                  GLfixed ffaend = span.alpha + len * span.alphaStep;
                  if (ffaend < 0) {
                     span.alpha -= ffaend;
                     if (span.alpha < 0)
                        span.alpha = 0;
                  }
               }

               /* This is where we actually generate fragments */
               if (span.end > 0) {
                  span.interpMask &= ~0x001; span.arrayMask |= 0x001; fast_persp_span(ctx, &span, &info);;
               }

               /*
                * Advance to the next scan line.  Compute the
                * new edge coordinates, and adjust the
                * pixel-center x coordinate so that it stays
                * on or inside the major edge.
                */
               (span.y)++;
               lines--;

               fxLeftEdge += fdxLeftEdge;
               fxRightEdge += fdxRightEdge;


               fError += fdError;
               if (fError >= 0) {
                  fError -= 0x00000800;
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowOuter);
                  fz += fdzOuter;
                  fogLeft += dfogOuter;
                  fr += fdrOuter;
                  fg += fdgOuter;
                  fb += fdbOuter;
                  fa += fdaOuter;
                  sLeft += dsOuter;
                  tLeft += dtOuter;
                  uLeft += duOuter;
                  vLeft += dvOuter;
               }
               else {
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowInner);
                  fz += fdzInner;
                  fogLeft += dfogInner;
                  fr += fdrInner;
                  fg += fdgInner;
                  fb += fdbInner;
                  fa += fdaInner;
                  sLeft += dsInner;
                  tLeft += dtInner;
                  uLeft += duInner;
                  vLeft += dvInner;
               }
            } /*while lines>0*/

         } /* for subTriangle */

      }
   }
}




/*
 * Render a smooth-shaded, textured, RGBA triangle.
 * Interpolate S,T,R with perspective correction, w/out mipmapping.
 */
/*
 * Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom triangle rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated across the triangle:
 *    INTERP_Z        - if defined, interpolate Z values
 *    INTERP_FOG      - if defined, interpolate fog values
 *    INTERP_RGB      - if defined, interpolate RGB values
 *    INTERP_ALPHA    - if defined, interpolate Alpha values (req's INTERP_RGB)
 *    INTERP_SPEC     - if defined, interpolate specular RGB values
 *    INTERP_INDEX    - if defined, interpolate color index values
 *    INTERP_INT_TEX  - if defined, interpolate integer ST texcoords
 *                         (fast, simple 2-D texture mapping)
 *    INTERP_TEX      - if defined, interpolate set 0 float STRQ texcoords
 *                         NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_MULTITEX - if defined, interpolate N units of STRQ texcoords
 *    INTERP_FLOAT_RGBA - if defined, interpolate RGBA with floating point
 *    INTERP_FLOAT_SPEC - if defined, interpolate specular with floating point
 *
 * When one can directly address pixels in the color buffer the following
 * macros can be defined and used to compute pixel addresses during
 * rasterization (see pRow):
 *    PIXEL_TYPE          - the datatype of a pixel (GLubyte, GLushort, GLuint)
 *    BYTES_PER_ROW       - number of bytes per row in the color buffer
 *    PIXEL_ADDRESS(X,Y)  - returns the address of pixel at (X,Y) where
 *                          Y==0 at bottom of screen and increases upward.
 *
 * Similarly, for direct depth buffer access, this type is used for depth
 * buffer addressing:
 *    DEPTH_TYPE          - either GLushort or GLuint
 *
 * Optionally, one may provide one-time setup code per triangle:
 *    SETUP_CODE    - code which is to be executed once per triangle
 *    CLEANUP_CODE    - code to execute at end of triangle
 *
 * The following macro MUST be defined:
 *    RENDER_SPAN(span) - code to write a span of pixels.
 *
 * This code was designed for the origin to be in the lower-left corner.
 *
 * Inspired by triangle rasterizer code written by Allen Akin.  Thanks Allen!
 */


/*
 * This is a bit of a hack, but it's a centralized place to enable floating-
 * point color interpolation when GLchan is actually floating point.
 */



static void general_textured_triangle(GLcontext *ctx, const SWvertex *v0,
                                 const SWvertex *v1,
                                 const SWvertex *v2 )
{
   typedef struct {
        const SWvertex *v0, *v1;   /* Y(v0) < Y(v1) */
        GLfloat dx;    /* X(v1) - X(v0) */
        GLfloat dy;    /* Y(v1) - Y(v0) */
        GLfixed fdxdy; /* dx/dy in fixed-point */
        GLfixed fsx;   /* first sample point x coord */
        GLfixed fsy;
        GLfloat adjy;  /* adjust from v[0]->fy to fsy, scaled */
        GLint lines;   /* number of lines to be sampled on this edge */
        GLfixed fx0;   /* fixed pt X of lower endpoint */
   } EdgeT;

   const GLint depthBits = ctx->Visual.depthBits;
   const GLfloat maxDepth = ctx->DepthMaxF;
   EdgeT eMaj, eTop, eBot;
   GLfloat oneOverArea;
   const SWvertex *vMin, *vMid, *vMax;  /* Y(vMin)<=Y(vMid)<=Y(vMax) */
   float bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;
   const GLint snapMask = ~((0x00000800 / (1 << 4)) - 1); /* for x/y coord snapping */
   GLfixed vMin_fx, vMin_fy, vMid_fx, vMid_fy, vMax_fx, vMax_fy;

   struct sw_span span;

   (span).primitive = (0x0009);
   (span).interpMask = (0);
   (span).arrayMask = (0);
   (span).start = 0;
   (span).end = (0);
   (span).facing = 0;
   (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays;


   /*
   printf("%s()\n", __FUNCTION__);
   printf("  %g, %g, %g\n", v0->win[0], v0->win[1], v0->win[2]);
   printf("  %g, %g, %g\n", v1->win[0], v1->win[1], v1->win[2]);
   printf("  %g, %g, %g\n", v2->win[0], v2->win[1], v2->win[2]);
    */

   /* Compute fixed point x,y coords w/ half-pixel offsets and snapping.
    * And find the order of the 3 vertices along the Y axis.
    */
   {
      const GLfixed fy0 = (((int) ((((v0->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v0->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v0->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy1 = (((int) ((((v1->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v1->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v1->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy2 = (((int) ((((v2->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v2->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v2->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;

      if (fy0 <= fy1) {
         if (fy1 <= fy2) {
            /* y0 <= y1 <= y2 */
            vMin = v0;   vMid = v1;   vMax = v2;
            vMin_fy = fy0;  vMid_fy = fy1;  vMax_fy = fy2;
         }
         else if (fy2 <= fy0) {
            /* y2 <= y0 <= y1 */
            vMin = v2;   vMid = v0;   vMax = v1;
            vMin_fy = fy2;  vMid_fy = fy0;  vMax_fy = fy1;
         }
         else {
            /* y0 <= y2 <= y1 */
            vMin = v0;   vMid = v2;   vMax = v1;
            vMin_fy = fy0;  vMid_fy = fy2;  vMax_fy = fy1;
            bf = -bf;
         }
      }
      else {
         if (fy0 <= fy2) {
            /* y1 <= y0 <= y2 */
            vMin = v1;   vMid = v0;   vMax = v2;
            vMin_fy = fy1;  vMid_fy = fy0;  vMax_fy = fy2;
            bf = -bf;
         }
         else if (fy2 <= fy1) {
            /* y2 <= y1 <= y0 */
            vMin = v2;   vMid = v1;   vMax = v0;
            vMin_fy = fy2;  vMid_fy = fy1;  vMax_fy = fy0;
            bf = -bf;
         }
         else {
            /* y1 <= y2 <= y0 */
            vMin = v1;   vMid = v2;   vMax = v0;
            vMin_fy = fy1;  vMid_fy = fy2;  vMax_fy = fy0;
         }
      }

      /* fixed point X coords */
      vMin_fx = (((int) ((((vMin->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMin->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMin->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMid_fx = (((int) ((((vMid->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMid->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMid->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMax_fx = (((int) ((((vMax->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMax->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMax->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
   }

   /* vertex/edge relationship */
   eMaj.v0 = vMin;   eMaj.v1 = vMax;   /*TODO: .v1's not needed */
   eTop.v0 = vMid;   eTop.v1 = vMax;
   eBot.v0 = vMin;   eBot.v1 = vMid;

   /* compute deltas for each edge:  vertex[upper] - vertex[lower] */
   eMaj.dx = ((vMax_fx - vMin_fx) * (1.0F / 2048.0f));
   eMaj.dy = ((vMax_fy - vMin_fy) * (1.0F / 2048.0f));
   eTop.dx = ((vMax_fx - vMid_fx) * (1.0F / 2048.0f));
   eTop.dy = ((vMax_fy - vMid_fy) * (1.0F / 2048.0f));
   eBot.dx = ((vMid_fx - vMin_fx) * (1.0F / 2048.0f));
   eBot.dy = ((vMid_fy - vMin_fy) * (1.0F / 2048.0f));

   /* compute area, oneOverArea and perform backface culling */
   {
      const GLfloat area = eMaj.dx * eBot.dy - eBot.dx * eMaj.dy;

      /* Do backface culling */
      if (area * bf < 0.0)
         return;

      if (IS_INF_OR_NAN(area) || area == 0.0F)
         return;

      oneOverArea = 1.0F / area;
   }

   ctx->OcclusionResult = 0x1;
   span.facing = ctx->_Facing; /* for 2-sided stencil test */

   /* Edge setup.  For a triangle strip these could be reused... */
   {
      eMaj.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eMaj.lines = (((((vMax_fy - eMaj.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eMaj.lines > 0) {
         GLfloat dxdy = eMaj.dx / eMaj.dy;
         eMaj.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eMaj.adjy = (GLfloat) (eMaj.fsy - vMin_fy);  /* SCALED! */
         eMaj.fx0 = vMin_fx;
         eMaj.fsx = eMaj.fx0 + (GLfixed) (eMaj.adjy * dxdy);
      }
      else {
         return;  /*CULLED*/
      }

      eTop.fsy = (((vMid_fy) + 0x00000800 - 1) & (~0x000007FF));
      eTop.lines = (((((vMax_fy - eTop.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eTop.lines > 0) {
         GLfloat dxdy = eTop.dx / eTop.dy;
         eTop.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eTop.adjy = (GLfloat) (eTop.fsy - vMid_fy); /* SCALED! */
         eTop.fx0 = vMid_fx;
         eTop.fsx = eTop.fx0 + (GLfixed) (eTop.adjy * dxdy);
      }

      eBot.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eBot.lines = (((((vMid_fy - eBot.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eBot.lines > 0) {
         GLfloat dxdy = eBot.dx / eBot.dy;
         eBot.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eBot.adjy = (GLfloat) (eBot.fsy - vMin_fy);  /* SCALED! */
         eBot.fx0 = vMin_fx;
         eBot.fsx = eBot.fx0 + (GLfixed) (eBot.adjy * dxdy);
      }
   }

   /*
    * Conceptually, we view a triangle as two subtriangles
    * separated by a perfectly horizontal line.  The edge that is
    * intersected by this line is one with maximal absolute dy; we
    * call it a ``major'' edge.  The other two edges are the
    * ``top'' edge (for the upper subtriangle) and the ``bottom''
    * edge (for the lower subtriangle).  If either of these two
    * edges is horizontal or very close to horizontal, the
    * corresponding subtriangle might cover zero sample points;
    * we take care to handle such cases, for performance as well
    * as correctness.
    *
    * By stepping rasterization parameters along the major edge,
    * we can avoid recomputing them at the discontinuity where
    * the top and bottom edges meet.  However, this forces us to
    * be able to scan both left-to-right and right-to-left.
    * Also, we must determine whether the major edge is at the
    * left or right side of the triangle.  We do this by
    * computing the magnitude of the cross-product of the major
    * and top edges.  Since this magnitude depends on the sine of
    * the angle between the two edges, its sign tells us whether
    * we turn to the left or to the right when travelling along
    * the major edge to the top edge, and from this we infer
    * whether the major edge is on the left or the right.
    *
    * Serendipitously, this cross-product magnitude is also a
    * value we need to compute the iteration parameter
    * derivatives for the triangle, and it can be used to perform
    * backface culling because its sign tells us whether the
    * triangle is clockwise or counterclockwise.  In this code we
    * refer to it as ``area'' because it's also proportional to
    * the pixel area of the triangle.
    */

   {
      GLint scan_from_left_to_right;  /* true if scanning left-to-right */
      GLfloat dzdx, dzdy;
      GLfloat dfogdy;
      GLfloat drdx, drdy;
      GLfloat dgdx, dgdy;
      GLfloat dbdx, dbdy;
      GLfloat dadx, dady;
      GLfloat dsrdx, dsrdy;
      GLfloat dsgdx, dsgdy;
      GLfloat dsbdx, dsbdy;
      GLfloat dsdx, dsdy;
      GLfloat dtdx, dtdy;
      GLfloat dudx, dudy;
      GLfloat dvdx, dvdy;

      /*
       * Execute user-supplied setup code
       */

      scan_from_left_to_right = (oneOverArea < 0.0F);


      /* compute d?/dx and d?/dy derivatives */
      span.interpMask |= 0x008;
      {
         GLfloat eMaj_dz, eBot_dz;
         eMaj_dz = vMax->win[2] - vMin->win[2];
         eBot_dz = vMid->win[2] - vMin->win[2];
         dzdx = oneOverArea * (eMaj_dz * eBot.dy - eMaj.dy * eBot_dz);
         if (dzdx > maxDepth || dzdx < -maxDepth) {
            /* probably a sliver triangle */
            dzdx = 0.0;
            dzdy = 0.0;
         }
         else {
            dzdy = oneOverArea * (eMaj.dx * eBot_dz - eMaj_dz * eBot.dx);
         }
         if (depthBits <= 16)
            span.zStep = (((int) ((((dzdx) * 2048.0f) >= 0.0F) ? (((dzdx) * 2048.0f) + 0.5F) : (((dzdx) * 2048.0f) - 0.5F))));
         else
            span.zStep = (GLint) dzdx;
      }
      span.interpMask |= 0x010;
      {
         const GLfloat eMaj_dfog = vMax->fog - vMin->fog;
         const GLfloat eBot_dfog = vMid->fog - vMin->fog;
         span.fogStep = oneOverArea * (eMaj_dfog * eBot.dy - eMaj.dy * eBot_dfog);
         dfogdy = oneOverArea * (eMaj.dx * eBot_dfog - eMaj_dfog * eBot.dx);
      }
      span.interpMask |= 0x001;
      if (ctx->Light.ShadeModel == 0x1D01) {
         GLfloat eMaj_dr, eBot_dr;
         GLfloat eMaj_dg, eBot_dg;
         GLfloat eMaj_db, eBot_db;
         GLfloat eMaj_da, eBot_da;
         eMaj_dr = (GLfloat) ((GLint) vMax->color[0] -
                             (GLint) vMin->color[0]);
         eBot_dr = (GLfloat) ((GLint) vMid->color[0] -
                             (GLint) vMin->color[0]);
         drdx = oneOverArea * (eMaj_dr * eBot.dy - eMaj.dy * eBot_dr);
         span.redStep = (((int) ((((drdx) * 2048.0f) >= 0.0F) ? (((drdx) * 2048.0f) + 0.5F) : (((drdx) * 2048.0f) - 0.5F))));
         drdy = oneOverArea * (eMaj.dx * eBot_dr - eMaj_dr * eBot.dx);
         eMaj_dg = (GLfloat) ((GLint) vMax->color[1] -
                             (GLint) vMin->color[1]);
         eBot_dg = (GLfloat) ((GLint) vMid->color[1] -
                             (GLint) vMin->color[1]);
         dgdx = oneOverArea * (eMaj_dg * eBot.dy - eMaj.dy * eBot_dg);
         span.greenStep = (((int) ((((dgdx) * 2048.0f) >= 0.0F) ? (((dgdx) * 2048.0f) + 0.5F) : (((dgdx) * 2048.0f) - 0.5F))));
         dgdy = oneOverArea * (eMaj.dx * eBot_dg - eMaj_dg * eBot.dx);
         eMaj_db = (GLfloat) ((GLint) vMax->color[2] -
                             (GLint) vMin->color[2]);
         eBot_db = (GLfloat) ((GLint) vMid->color[2] -
                             (GLint) vMin->color[2]);
         dbdx = oneOverArea * (eMaj_db * eBot.dy - eMaj.dy * eBot_db);
         span.blueStep = (((int) ((((dbdx) * 2048.0f) >= 0.0F) ? (((dbdx) * 2048.0f) + 0.5F) : (((dbdx) * 2048.0f) - 0.5F))));
         dbdy = oneOverArea * (eMaj.dx * eBot_db - eMaj_db * eBot.dx);
         eMaj_da = (GLfloat) ((GLint) vMax->color[3] -
                             (GLint) vMin->color[3]);
         eBot_da = (GLfloat) ((GLint) vMid->color[3] -
                             (GLint) vMin->color[3]);
         dadx = oneOverArea * (eMaj_da * eBot.dy - eMaj.dy * eBot_da);
         span.alphaStep = (((int) ((((dadx) * 2048.0f) >= 0.0F) ? (((dadx) * 2048.0f) + 0.5F) : (((dadx) * 2048.0f) - 0.5F))));
         dady = oneOverArea * (eMaj.dx * eBot_da - eMaj_da * eBot.dx);
      }
      else {
         ;
         span.interpMask |= 0x200;
         drdx = drdy = 0.0F;
         dgdx = dgdy = 0.0F;
         dbdx = dbdy = 0.0F;
         span.redStep = 0;
         span.greenStep = 0;
         span.blueStep = 0;
         dadx = dady = 0.0F;
         span.alphaStep = 0;
      }
      span.interpMask |= 0x002;
      if (ctx->Light.ShadeModel == 0x1D01) {
         GLfloat eMaj_dsr, eBot_dsr;
         GLfloat eMaj_dsg, eBot_dsg;
         GLfloat eMaj_dsb, eBot_dsb;
         eMaj_dsr = (GLfloat) ((GLint) vMax->specular[0] -
                              (GLint) vMin->specular[0]);
         eBot_dsr = (GLfloat) ((GLint) vMid->specular[0] -
                              (GLint) vMin->specular[0]);
         dsrdx = oneOverArea * (eMaj_dsr * eBot.dy - eMaj.dy * eBot_dsr);
         span.specRedStep = (((int) ((((dsrdx) * 2048.0f) >= 0.0F) ? (((dsrdx) * 2048.0f) + 0.5F) : (((dsrdx) * 2048.0f) - 0.5F))));
         dsrdy = oneOverArea * (eMaj.dx * eBot_dsr - eMaj_dsr * eBot.dx);
         eMaj_dsg = (GLfloat) ((GLint) vMax->specular[1] -
                              (GLint) vMin->specular[1]);
         eBot_dsg = (GLfloat) ((GLint) vMid->specular[1] -
                              (GLint) vMin->specular[1]);
         dsgdx = oneOverArea * (eMaj_dsg * eBot.dy - eMaj.dy * eBot_dsg);
         span.specGreenStep = (((int) ((((dsgdx) * 2048.0f) >= 0.0F) ? (((dsgdx) * 2048.0f) + 0.5F) : (((dsgdx) * 2048.0f) - 0.5F))));
         dsgdy = oneOverArea * (eMaj.dx * eBot_dsg - eMaj_dsg * eBot.dx);
         eMaj_dsb = (GLfloat) ((GLint) vMax->specular[2] -
                              (GLint) vMin->specular[2]);
         eBot_dsb = (GLfloat) ((GLint) vMid->specular[2] -
                              (GLint) vMin->specular[2]);
         dsbdx = oneOverArea * (eMaj_dsb * eBot.dy - eMaj.dy * eBot_dsb);
         span.specBlueStep = (((int) ((((dsbdx) * 2048.0f) >= 0.0F) ? (((dsbdx) * 2048.0f) + 0.5F) : (((dsbdx) * 2048.0f) - 0.5F))));
         dsbdy = oneOverArea * (eMaj.dx * eBot_dsb - eMaj_dsb * eBot.dx);
      }
      else {
         dsrdx = dsrdy = 0.0F;
         dsgdx = dsgdy = 0.0F;
         dsbdx = dsbdy = 0.0F;
         span.specRedStep = 0;
         span.specGreenStep = 0;
         span.specBlueStep = 0;
      }
      span.interpMask |= 0x020;
      {
         GLfloat wMax = vMax->win[3];
         GLfloat wMin = vMin->win[3];
         GLfloat wMid = vMid->win[3];
         GLfloat eMaj_ds, eBot_ds;
         GLfloat eMaj_dt, eBot_dt;
         GLfloat eMaj_du, eBot_du;
         GLfloat eMaj_dv, eBot_dv;

         eMaj_ds = vMax->texcoord[0][0] * wMax - vMin->texcoord[0][0] * wMin;
         eBot_ds = vMid->texcoord[0][0] * wMid - vMin->texcoord[0][0] * wMin;
         dsdx = oneOverArea * (eMaj_ds * eBot.dy - eMaj.dy * eBot_ds);
         dsdy = oneOverArea * (eMaj.dx * eBot_ds - eMaj_ds * eBot.dx);
         span.texStepX[0][0] = dsdx;
         span.texStepY[0][0] = dsdy;

         eMaj_dt = vMax->texcoord[0][1] * wMax - vMin->texcoord[0][1] * wMin;
         eBot_dt = vMid->texcoord[0][1] * wMid - vMin->texcoord[0][1] * wMin;
         dtdx = oneOverArea * (eMaj_dt * eBot.dy - eMaj.dy * eBot_dt);
         dtdy = oneOverArea * (eMaj.dx * eBot_dt - eMaj_dt * eBot.dx);
         span.texStepX[0][1] = dtdx;
         span.texStepY[0][1] = dtdy;

         eMaj_du = vMax->texcoord[0][2] * wMax - vMin->texcoord[0][2] * wMin;
         eBot_du = vMid->texcoord[0][2] * wMid - vMin->texcoord[0][2] * wMin;
         dudx = oneOverArea * (eMaj_du * eBot.dy - eMaj.dy * eBot_du);
         dudy = oneOverArea * (eMaj.dx * eBot_du - eMaj_du * eBot.dx);
         span.texStepX[0][2] = dudx;
         span.texStepY[0][2] = dudy;

         eMaj_dv = vMax->texcoord[0][3] * wMax - vMin->texcoord[0][3] * wMin;
         eBot_dv = vMid->texcoord[0][3] * wMid - vMin->texcoord[0][3] * wMin;
         dvdx = oneOverArea * (eMaj_dv * eBot.dy - eMaj.dy * eBot_dv);
         dvdy = oneOverArea * (eMaj.dx * eBot_dv - eMaj_dv * eBot.dx);
         span.texStepX[0][3] = dvdx;
         span.texStepY[0][3] = dvdy;
      }

      /*
       * We always sample at pixel centers.  However, we avoid
       * explicit half-pixel offsets in this code by incorporating
       * the proper offset in each of x and y during the
       * transformation to window coordinates.
       *
       * We also apply the usual rasterization rules to prevent
       * cracks and overlaps.  A pixel is considered inside a
       * subtriangle if it meets all of four conditions: it is on or
       * to the right of the left edge, strictly to the left of the
       * right edge, on or below the top edge, and strictly above
       * the bottom edge.  (Some edges may be degenerate.)
       *
       * The following discussion assumes left-to-right scanning
       * (that is, the major edge is on the left); the right-to-left
       * case is a straightforward variation.
       *
       * We start by finding the half-integral y coordinate that is
       * at or below the top of the triangle.  This gives us the
       * first scan line that could possibly contain pixels that are
       * inside the triangle.
       *
       * Next we creep down the major edge until we reach that y,
       * and compute the corresponding x coordinate on the edge.
       * Then we find the half-integral x that lies on or just
       * inside the edge.  This is the first pixel that might lie in
       * the interior of the triangle.  (We won't know for sure
       * until we check the other edges.)
       *
       * As we rasterize the triangle, we'll step down the major
       * edge.  For each step in y, we'll move an integer number
       * of steps in x.  There are two possible x step sizes, which
       * we'll call the ``inner'' step (guaranteed to land on the
       * edge or inside it) and the ``outer'' step (guaranteed to
       * land on the edge or outside it).  The inner and outer steps
       * differ by one.  During rasterization we maintain an error
       * term that indicates our distance from the true edge, and
       * select either the inner step or the outer step, whichever
       * gets us to the first pixel that falls inside the triangle.
       *
       * All parameters (z, red, etc.) as well as the buffer
       * addresses for color and z have inner and outer step values,
       * so that we can increment them appropriately.  This method
       * eliminates the need to adjust parameters by creeping a
       * sub-pixel amount into the triangle at each scanline.
       */

      {
         int subTriangle;
         GLfixed fx;
         GLfixed fxLeftEdge = 0, fxRightEdge = 0;
         GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
         GLfixed fdxOuter;
         int idxOuter;
         float dxOuter;
         GLfixed fError = 0, fdError = 0;
         float adjx, adjy;
         GLfixed fy;
         GLushort *zRow = 0;
         int dZRowOuter = 0, dZRowInner;  /* offset in bytes */
         GLfixed fz = 0, fdzOuter = 0, fdzInner;
         GLfloat fogLeft = 0, dfogOuter = 0, dfogInner;
         GLfixed fr = 0, fdrOuter = 0, fdrInner;
         GLfixed fg = 0, fdgOuter = 0, fdgInner;
         GLfixed fb = 0, fdbOuter = 0, fdbInner;
         GLfixed fa = 0, fdaOuter = 0, fdaInner;
         GLfixed fsr=0, fdsrOuter=0, fdsrInner;
         GLfixed fsg=0, fdsgOuter=0, fdsgInner;
         GLfixed fsb=0, fdsbOuter=0, fdsbInner;
         GLfloat sLeft=0, dsOuter=0, dsInner;
         GLfloat tLeft=0, dtOuter=0, dtInner;
         GLfloat uLeft=0, duOuter=0, duInner;
         GLfloat vLeft=0, dvOuter=0, dvInner;

         for (subTriangle=0; subTriangle<=1; subTriangle++) {
            EdgeT *eLeft, *eRight;
            int setupLeft, setupRight;
            int lines;

            if (subTriangle==0) {
               /* bottom half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eBot;
                  lines = eRight->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
               else {
                  eLeft = &eBot;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
            }
            else {
               /* top half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eTop;
                  lines = eRight->lines;
                  setupLeft = 0;
                  setupRight = 1;
               }
               else {
                  eLeft = &eTop;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 0;
               }
               if (lines == 0)
                  return;
            }

            if (setupLeft && eLeft->lines > 0) {
               const SWvertex *vLower;
               GLfixed fsx = eLeft->fsx;
               fx = (((fsx) + 0x00000800 - 1) & (~0x000007FF));
               fError = fx - fsx - 0x00000800;
               fxLeftEdge = fsx - 1;
               fdxLeftEdge = eLeft->fdxdy;
               fdxOuter = ((fdxLeftEdge - 1) & (~0x000007FF));
               fdError = fdxOuter - fdxLeftEdge + 0x00000800;
               idxOuter = ((fdxOuter) >> 11);
               dxOuter = (float) idxOuter;
               (void) dxOuter;

               fy = eLeft->fsy;
               span.y = ((fy) >> 11);

               adjx = (float)(fx - eLeft->fx0);  /* SCALED! */
               adjy = eLeft->adjy;              /* SCALED! */
               vLower = eLeft->v0;

               /*
                * Now we need the set of parameter (z, color, etc.) values at
                * the point (fx, fy).  This gives us properly-sampled parameter
                * values that we can step from pixel to pixel.  Furthermore,
                * although we might have intermediate results that overflow
                * the normal parameter range when we step temporarily outside
                * the triangle, we shouldn't overflow or underflow for any
                * pixel that's actually inside the triangle.
                */

               {
                  GLfloat z0 = vLower->win[2];
                  if (depthBits <= 16) {
                     /* interpolate fixed-pt values */
                     GLfloat tmp = (z0 * 2048.0f +
                                    dzdx * adjx + dzdy * adjy) + 0x00000400;
                     if (tmp < 0xffffffff / 2)
                        fz = (GLfixed) tmp;
                     else
                        fz = 0xffffffff / 2;
                     fdzOuter = (((int) ((((dzdy + dxOuter * dzdx) * 2048.0f) >= 0.0F) ? (((dzdy + dxOuter * dzdx) * 2048.0f) + 0.5F) : (((dzdy + dxOuter * dzdx) * 2048.0f) - 0.5F))));
                  }
                  else {
                     /* interpolate depth values exactly */
                     fz = (GLint) (z0 + dzdx * ((adjx) * (1.0F / 2048.0f))
                                   + dzdy * ((adjy) * (1.0F / 2048.0f)));
                     fdzOuter = (GLint) (dzdy + dxOuter * dzdx);
                  }
                  zRow = (GLushort *)
                    _mesa_zbuffer_address(ctx, ((fxLeftEdge) >> 11), span.y);
                  dZRowOuter = (ctx->DrawBuffer->Width + idxOuter) * sizeof(GLushort);
               }
               fogLeft = vLower->fog + (span.fogStep * adjx + dfogdy * adjy)
                                       * (1.0F/2048.0f);
               dfogOuter = dfogdy + dxOuter * span.fogStep;
               if (ctx->Light.ShadeModel == 0x1D01) {
                  fr = (GLfixed) (((vLower->color[0]) << 11)
                                   + drdx * adjx + drdy * adjy) + 0x00000400;
                  fdrOuter = (((int) ((((drdy + dxOuter * drdx) * 2048.0f) >= 0.0F) ? (((drdy + dxOuter * drdx) * 2048.0f) + 0.5F) : (((drdy + dxOuter * drdx) * 2048.0f) - 0.5F))));
                  fg = (GLfixed) (((vLower->color[1]) << 11)
                                   + dgdx * adjx + dgdy * adjy) + 0x00000400;
                  fdgOuter = (((int) ((((dgdy + dxOuter * dgdx) * 2048.0f) >= 0.0F) ? (((dgdy + dxOuter * dgdx) * 2048.0f) + 0.5F) : (((dgdy + dxOuter * dgdx) * 2048.0f) - 0.5F))));
                  fb = (GLfixed) (((vLower->color[2]) << 11)
                                    + dbdx * adjx + dbdy * adjy) + 0x00000400;
                  fdbOuter = (((int) ((((dbdy + dxOuter * dbdx) * 2048.0f) >= 0.0F) ? (((dbdy + dxOuter * dbdx) * 2048.0f) + 0.5F) : (((dbdy + dxOuter * dbdx) * 2048.0f) - 0.5F))));
                  fa = (GLfixed) (((vLower->color[3]) << 11)
                                   + dadx * adjx + dady * adjy) + 0x00000400;
                  fdaOuter = (((int) ((((dady + dxOuter * dadx) * 2048.0f) >= 0.0F) ? (((dady + dxOuter * dadx) * 2048.0f) + 0.5F) : (((dady + dxOuter * dadx) * 2048.0f) - 0.5F))));
               }
               else {
                  ;
                  fr = ((v2->color[0]) << 11);
                  fg = ((v2->color[1]) << 11);
                  fb = ((v2->color[2]) << 11);
                  fdrOuter = fdgOuter = fdbOuter = 0;
                  fa =  ((v2->color[3]) << 11);
                  fdaOuter = 0;
               }
               if (ctx->Light.ShadeModel == 0x1D01) {
                  fsr = (GLfixed) (((vLower->specular[0]) << 11)
                                   + dsrdx * adjx + dsrdy * adjy) + 0x00000400;
                  fdsrOuter = (((int) ((((dsrdy + dxOuter * dsrdx) * 2048.0f) >= 0.0F) ? (((dsrdy + dxOuter * dsrdx) * 2048.0f) + 0.5F) : (((dsrdy + dxOuter * dsrdx) * 2048.0f) - 0.5F))));
                  fsg = (GLfixed) (((vLower->specular[1]) << 11)
                                   + dsgdx * adjx + dsgdy * adjy) + 0x00000400;
                  fdsgOuter = (((int) ((((dsgdy + dxOuter * dsgdx) * 2048.0f) >= 0.0F) ? (((dsgdy + dxOuter * dsgdx) * 2048.0f) + 0.5F) : (((dsgdy + dxOuter * dsgdx) * 2048.0f) - 0.5F))));
                  fsb = (GLfixed) (((vLower->specular[2]) << 11)
                                   + dsbdx * adjx + dsbdy * adjy) + 0x00000400;
                  fdsbOuter = (((int) ((((dsbdy + dxOuter * dsbdx) * 2048.0f) >= 0.0F) ? (((dsbdy + dxOuter * dsbdx) * 2048.0f) + 0.5F) : (((dsbdy + dxOuter * dsbdx) * 2048.0f) - 0.5F))));
               }
               else {
                  fsr = ((v2->specular[0]) << 11);
                  fsg = ((v2->specular[1]) << 11);
                  fsb = ((v2->specular[2]) << 11);
                  fdsrOuter = fdsgOuter = fdsbOuter = 0;
               }
               {
                  GLfloat invW = vLower->win[3];
                  GLfloat s0, t0, u0, v0;
                  s0 = vLower->texcoord[0][0] * invW;
                  sLeft = s0 + (span.texStepX[0][0] * adjx + dsdy * adjy)
                     * (1.0F/2048.0f);
                  dsOuter = dsdy + dxOuter * span.texStepX[0][0];
                  t0 = vLower->texcoord[0][1] * invW;
                  tLeft = t0 + (span.texStepX[0][1] * adjx + dtdy * adjy)
                     * (1.0F/2048.0f);
                  dtOuter = dtdy + dxOuter * span.texStepX[0][1];
                  u0 = vLower->texcoord[0][2] * invW;
                  uLeft = u0 + (span.texStepX[0][2] * adjx + dudy * adjy)
                     * (1.0F/2048.0f);
                  duOuter = dudy + dxOuter * span.texStepX[0][2];
                  v0 = vLower->texcoord[0][3] * invW;
                  vLeft = v0 + (span.texStepX[0][3] * adjx + dvdy * adjy)
                     * (1.0F/2048.0f);
                  dvOuter = dvdy + dxOuter * span.texStepX[0][3];
               }

            } /*if setupLeft*/


            if (setupRight && eRight->lines>0) {
               fxRightEdge = eRight->fsx - 1;
               fdxRightEdge = eRight->fdxdy;
            }

            if (lines==0) {
               continue;
            }


            /* Rasterize setup */
            dZRowInner = dZRowOuter + sizeof(GLushort);
            fdzInner = fdzOuter + span.zStep;
            dfogInner = dfogOuter + span.fogStep;
            fdrInner = fdrOuter + span.redStep;
            fdgInner = fdgOuter + span.greenStep;
            fdbInner = fdbOuter + span.blueStep;
            fdaInner = fdaOuter + span.alphaStep;
            fdsrInner = fdsrOuter + span.specRedStep;
            fdsgInner = fdsgOuter + span.specGreenStep;
            fdsbInner = fdsbOuter + span.specBlueStep;
            dsInner = dsOuter + span.texStepX[0][0];
            dtInner = dtOuter + span.texStepX[0][1];
            duInner = duOuter + span.texStepX[0][2];
            dvInner = dvOuter + span.texStepX[0][3];

            while (lines > 0) {
               /* initialize the span interpolants to the leftmost value */
               /* ff = fixed-pt fragment */
               const GLint right = ((fxRightEdge) >> 11);

               span.x = ((fxLeftEdge) >> 11);

               if (right <= span.x)
                  span.end = 0;
               else
                  span.end = right - span.x;

               span.z = fz;
               span.fog = fogLeft;
               span.red = fr;
               span.green = fg;
               span.blue = fb;
               span.alpha = fa;
               span.specRed = fsr;
               span.specGreen = fsg;
               span.specBlue = fsb;

               span.tex[0][0] = sLeft;
               span.tex[0][1] = tLeft;
               span.tex[0][2] = uLeft;
               span.tex[0][3] = vLeft;


               {
                  /* need this to accomodate round-off errors */
                  const GLint len = right - span.x - 1;
                  GLfixed ffrend = span.red + len * span.redStep;
                  GLfixed ffgend = span.green + len * span.greenStep;
                  GLfixed ffbend = span.blue + len * span.blueStep;
                  if (ffrend < 0) {
                     span.red -= ffrend;
                     if (span.red < 0)
                        span.red = 0;
                  }
                  if (ffgend < 0) {
                     span.green -= ffgend;
                     if (span.green < 0)
                        span.green = 0;
                  }
                  if (ffbend < 0) {
                     span.blue -= ffbend;
                     if (span.blue < 0)
                        span.blue = 0;
                  }
               }
               {
                  const GLint len = right - span.x - 1;
                  GLfixed ffaend = span.alpha + len * span.alphaStep;
                  if (ffaend < 0) {
                     span.alpha -= ffaend;
                     if (span.alpha < 0)
                        span.alpha = 0;
                  }
               }
               {
                  /* need this to accomodate round-off errors */
                  const GLint len = right - span.x - 1;
                  GLfixed ffsrend = span.specRed + len * span.specRedStep;
                  GLfixed ffsgend = span.specGreen + len * span.specGreenStep;
                  GLfixed ffsbend = span.specBlue + len * span.specBlueStep;
                  if (ffsrend < 0) {
                     span.specRed -= ffsrend;
                     if (span.specRed < 0)
                        span.specRed = 0;
                  }
                  if (ffsgend < 0) {
                     span.specGreen -= ffsgend;
                     if (span.specGreen < 0)
                        span.specGreen = 0;
                  }
                  if (ffsbend < 0) {
                     span.specBlue -= ffsbend;
                     if (span.specBlue < 0)
                        span.specBlue = 0;
                  }
               }

               /* This is where we actually generate fragments */
               if (span.end > 0) {
                  _mesa_write_texture_span(ctx, &span);;
               }

               /*
                * Advance to the next scan line.  Compute the
                * new edge coordinates, and adjust the
                * pixel-center x coordinate so that it stays
                * on or inside the major edge.
                */
               (span.y)++;
               lines--;

               fxLeftEdge += fdxLeftEdge;
               fxRightEdge += fdxRightEdge;


               fError += fdError;
               if (fError >= 0) {
                  fError -= 0x00000800;
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowOuter);
                  fz += fdzOuter;
                  fogLeft += dfogOuter;
                  fr += fdrOuter;
                  fg += fdgOuter;
                  fb += fdbOuter;
                  fa += fdaOuter;
                  fsr += fdsrOuter;
                  fsg += fdsgOuter;
                  fsb += fdsbOuter;
                  sLeft += dsOuter;
                  tLeft += dtOuter;
                  uLeft += duOuter;
                  vLeft += dvOuter;
               }
               else {
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowInner);
                  fz += fdzInner;
                  fogLeft += dfogInner;
                  fr += fdrInner;
                  fg += fdgInner;
                  fb += fdbInner;
                  fa += fdaInner;
                  fsr += fdsrInner;
                  fsg += fdsgInner;
                  fsb += fdsbInner;
                  sLeft += dsInner;
                  tLeft += dtInner;
                  uLeft += duInner;
                  vLeft += dvInner;
               }
            } /*while lines>0*/

         } /* for subTriangle */

      }
   }
}



/*
 * This is the big one!
 * Interpolate Z, RGB, Alpha, specular, fog, and N sets of texture coordinates.
 * Yup, it's slow.
 */

/*
 * Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom triangle rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated across the triangle:
 *    INTERP_Z        - if defined, interpolate Z values
 *    INTERP_FOG      - if defined, interpolate fog values
 *    INTERP_RGB      - if defined, interpolate RGB values
 *    INTERP_ALPHA    - if defined, interpolate Alpha values (req's INTERP_RGB)
 *    INTERP_SPEC     - if defined, interpolate specular RGB values
 *    INTERP_INDEX    - if defined, interpolate color index values
 *    INTERP_INT_TEX  - if defined, interpolate integer ST texcoords
 *                         (fast, simple 2-D texture mapping)
 *    INTERP_TEX      - if defined, interpolate set 0 float STRQ texcoords
 *                         NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_MULTITEX - if defined, interpolate N units of STRQ texcoords
 *    INTERP_FLOAT_RGBA - if defined, interpolate RGBA with floating point
 *    INTERP_FLOAT_SPEC - if defined, interpolate specular with floating point
 *
 * When one can directly address pixels in the color buffer the following
 * macros can be defined and used to compute pixel addresses during
 * rasterization (see pRow):
 *    PIXEL_TYPE          - the datatype of a pixel (GLubyte, GLushort, GLuint)
 *    BYTES_PER_ROW       - number of bytes per row in the color buffer
 *    PIXEL_ADDRESS(X,Y)  - returns the address of pixel at (X,Y) where
 *                          Y==0 at bottom of screen and increases upward.
 *
 * Similarly, for direct depth buffer access, this type is used for depth
 * buffer addressing:
 *    DEPTH_TYPE          - either GLushort or GLuint
 *
 * Optionally, one may provide one-time setup code per triangle:
 *    SETUP_CODE    - code which is to be executed once per triangle
 *    CLEANUP_CODE    - code to execute at end of triangle
 *
 * The following macro MUST be defined:
 *    RENDER_SPAN(span) - code to write a span of pixels.
 *
 * This code was designed for the origin to be in the lower-left corner.
 *
 * Inspired by triangle rasterizer code written by Allen Akin.  Thanks Allen!
 */


/*
 * This is a bit of a hack, but it's a centralized place to enable floating-
 * point color interpolation when GLchan is actually floating point.
 */



static void multitextured_triangle(GLcontext *ctx, const SWvertex *v0,
                                 const SWvertex *v1,
                                 const SWvertex *v2 )
{
   typedef struct {
        const SWvertex *v0, *v1;   /* Y(v0) < Y(v1) */
        GLfloat dx;    /* X(v1) - X(v0) */
        GLfloat dy;    /* Y(v1) - Y(v0) */
        GLfixed fdxdy; /* dx/dy in fixed-point */
        GLfixed fsx;   /* first sample point x coord */
        GLfixed fsy;
        GLfloat adjy;  /* adjust from v[0]->fy to fsy, scaled */
        GLint lines;   /* number of lines to be sampled on this edge */
        GLfixed fx0;   /* fixed pt X of lower endpoint */
   } EdgeT;

   const GLint depthBits = ctx->Visual.depthBits;
   const GLfloat maxDepth = ctx->DepthMaxF;
   EdgeT eMaj, eTop, eBot;
   GLfloat oneOverArea;
   const SWvertex *vMin, *vMid, *vMax;  /* Y(vMin)<=Y(vMid)<=Y(vMax) */
   float bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;
   const GLint snapMask = ~((0x00000800 / (1 << 4)) - 1); /* for x/y coord snapping */
   GLfixed vMin_fx, vMin_fy, vMid_fx, vMid_fy, vMax_fx, vMax_fy;

   struct sw_span span;

   do { (span).primitive = (0x0009); (span).interpMask = (0); (span).arrayMask = (0); (span).start = 0; (span).end = (0); (span).facing = 0; (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays; } while (0);


   printf("%s()\n", __FUNCTION__);
   /*
   printf("  %g, %g, %g\n", v0->win[0], v0->win[1], v0->win[2]);
   printf("  %g, %g, %g\n", v1->win[0], v1->win[1], v1->win[2]);
   printf("  %g, %g, %g\n", v2->win[0], v2->win[1], v2->win[2]);
    */

   /* Compute fixed point x,y coords w/ half-pixel offsets and snapping.
    * And find the order of the 3 vertices along the Y axis.
    */
   {
      const GLfixed fy0 = (((int) ((((v0->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v0->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v0->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy1 = (((int) ((((v1->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v1->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v1->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy2 = (((int) ((((v2->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v2->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v2->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;

      if (fy0 <= fy1) {
         if (fy1 <= fy2) {
            /* y0 <= y1 <= y2 */
            vMin = v0;   vMid = v1;   vMax = v2;
            vMin_fy = fy0;  vMid_fy = fy1;  vMax_fy = fy2;
         }
         else if (fy2 <= fy0) {
            /* y2 <= y0 <= y1 */
            vMin = v2;   vMid = v0;   vMax = v1;
            vMin_fy = fy2;  vMid_fy = fy0;  vMax_fy = fy1;
         }
         else {
            /* y0 <= y2 <= y1 */
            vMin = v0;   vMid = v2;   vMax = v1;
            vMin_fy = fy0;  vMid_fy = fy2;  vMax_fy = fy1;
            bf = -bf;
         }
      }
      else {
         if (fy0 <= fy2) {
            /* y1 <= y0 <= y2 */
            vMin = v1;   vMid = v0;   vMax = v2;
            vMin_fy = fy1;  vMid_fy = fy0;  vMax_fy = fy2;
            bf = -bf;
         }
         else if (fy2 <= fy1) {
            /* y2 <= y1 <= y0 */
            vMin = v2;   vMid = v1;   vMax = v0;
            vMin_fy = fy2;  vMid_fy = fy1;  vMax_fy = fy0;
            bf = -bf;
         }
         else {
            /* y1 <= y2 <= y0 */
            vMin = v1;   vMid = v2;   vMax = v0;
            vMin_fy = fy1;  vMid_fy = fy2;  vMax_fy = fy0;
         }
      }

      /* fixed point X coords */
      vMin_fx = (((int) ((((vMin->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMin->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMin->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMid_fx = (((int) ((((vMid->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMid->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMid->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMax_fx = (((int) ((((vMax->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMax->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMax->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
   }

   /* vertex/edge relationship */
   eMaj.v0 = vMin;   eMaj.v1 = vMax;   /*TODO: .v1's not needed */
   eTop.v0 = vMid;   eTop.v1 = vMax;
   eBot.v0 = vMin;   eBot.v1 = vMid;

   /* compute deltas for each edge:  vertex[upper] - vertex[lower] */
   eMaj.dx = ((vMax_fx - vMin_fx) * (1.0F / 2048.0f));
   eMaj.dy = ((vMax_fy - vMin_fy) * (1.0F / 2048.0f));
   eTop.dx = ((vMax_fx - vMid_fx) * (1.0F / 2048.0f));
   eTop.dy = ((vMax_fy - vMid_fy) * (1.0F / 2048.0f));
   eBot.dx = ((vMid_fx - vMin_fx) * (1.0F / 2048.0f));
   eBot.dy = ((vMid_fy - vMin_fy) * (1.0F / 2048.0f));

   /* compute area, oneOverArea and perform backface culling */
   {
      const GLfloat area = eMaj.dx * eBot.dy - eBot.dx * eMaj.dy;

      /* Do backface culling */
      if (area * bf < 0.0)
         return;

      if (IS_INF_OR_NAN(area) || area == 0.0F)
         return;

      oneOverArea = 1.0F / area;
   }

   ctx->OcclusionResult = 0x1;
   span.facing = ctx->_Facing; /* for 2-sided stencil test */

   /* Edge setup.  For a triangle strip these could be reused... */
   {
      eMaj.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eMaj.lines = (((((vMax_fy - eMaj.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eMaj.lines > 0) {
         GLfloat dxdy = eMaj.dx / eMaj.dy;
         eMaj.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eMaj.adjy = (GLfloat) (eMaj.fsy - vMin_fy);  /* SCALED! */
         eMaj.fx0 = vMin_fx;
         eMaj.fsx = eMaj.fx0 + (GLfixed) (eMaj.adjy * dxdy);
      }
      else {
         return;  /*CULLED*/
      }

      eTop.fsy = (((vMid_fy) + 0x00000800 - 1) & (~0x000007FF));
      eTop.lines = (((((vMax_fy - eTop.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eTop.lines > 0) {
         GLfloat dxdy = eTop.dx / eTop.dy;
         eTop.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eTop.adjy = (GLfloat) (eTop.fsy - vMid_fy); /* SCALED! */
         eTop.fx0 = vMid_fx;
         eTop.fsx = eTop.fx0 + (GLfixed) (eTop.adjy * dxdy);
      }

      eBot.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eBot.lines = (((((vMid_fy - eBot.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eBot.lines > 0) {
         GLfloat dxdy = eBot.dx / eBot.dy;
         eBot.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eBot.adjy = (GLfloat) (eBot.fsy - vMin_fy);  /* SCALED! */
         eBot.fx0 = vMin_fx;
         eBot.fsx = eBot.fx0 + (GLfixed) (eBot.adjy * dxdy);
      }
   }

   /*
    * Conceptually, we view a triangle as two subtriangles
    * separated by a perfectly horizontal line.  The edge that is
    * intersected by this line is one with maximal absolute dy; we
    * call it a ``major'' edge.  The other two edges are the
    * ``top'' edge (for the upper subtriangle) and the ``bottom''
    * edge (for the lower subtriangle).  If either of these two
    * edges is horizontal or very close to horizontal, the
    * corresponding subtriangle might cover zero sample points;
    * we take care to handle such cases, for performance as well
    * as correctness.
    *
    * By stepping rasterization parameters along the major edge,
    * we can avoid recomputing them at the discontinuity where
    * the top and bottom edges meet.  However, this forces us to
    * be able to scan both left-to-right and right-to-left.
    * Also, we must determine whether the major edge is at the
    * left or right side of the triangle.  We do this by
    * computing the magnitude of the cross-product of the major
    * and top edges.  Since this magnitude depends on the sine of
    * the angle between the two edges, its sign tells us whether
    * we turn to the left or to the right when travelling along
    * the major edge to the top edge, and from this we infer
    * whether the major edge is on the left or the right.
    *
    * Serendipitously, this cross-product magnitude is also a
    * value we need to compute the iteration parameter
    * derivatives for the triangle, and it can be used to perform
    * backface culling because its sign tells us whether the
    * triangle is clockwise or counterclockwise.  In this code we
    * refer to it as ``area'' because it's also proportional to
    * the pixel area of the triangle.
    */

   {
      GLint scan_from_left_to_right;  /* true if scanning left-to-right */
      GLfloat dzdx, dzdy;
      GLfloat dfogdy;
      GLfloat drdx, drdy;
      GLfloat dgdx, dgdy;
      GLfloat dbdx, dbdy;
      GLfloat dadx, dady;
      GLfloat dsrdx, dsrdy;
      GLfloat dsgdx, dsgdy;
      GLfloat dsbdx, dsbdy;
      GLfloat dsdx[8], dsdy[8];
      GLfloat dtdx[8], dtdy[8];
      GLfloat dudx[8], dudy[8];
      GLfloat dvdx[8], dvdy[8];

      /*
       * Execute user-supplied setup code
       */

      scan_from_left_to_right = (oneOverArea < 0.0F);


      /* compute d?/dx and d?/dy derivatives */
      span.interpMask |= 0x008;
      {
         GLfloat eMaj_dz, eBot_dz;
         eMaj_dz = vMax->win[2] - vMin->win[2];
         eBot_dz = vMid->win[2] - vMin->win[2];
         dzdx = oneOverArea * (eMaj_dz * eBot.dy - eMaj.dy * eBot_dz);
         if (dzdx > maxDepth || dzdx < -maxDepth) {
            /* probably a sliver triangle */
            dzdx = 0.0;
            dzdy = 0.0;
         }
         else {
            dzdy = oneOverArea * (eMaj.dx * eBot_dz - eMaj_dz * eBot.dx);
         }
         if (depthBits <= 16)
            span.zStep = (((int) ((((dzdx) * 2048.0f) >= 0.0F) ? (((dzdx) * 2048.0f) + 0.5F) : (((dzdx) * 2048.0f) - 0.5F))));
         else
            span.zStep = (GLint) dzdx;
      }
      span.interpMask |= 0x010;
      {
         const GLfloat eMaj_dfog = vMax->fog - vMin->fog;
         const GLfloat eBot_dfog = vMid->fog - vMin->fog;
         span.fogStep = oneOverArea * (eMaj_dfog * eBot.dy - eMaj.dy * eBot_dfog);
         dfogdy = oneOverArea * (eMaj.dx * eBot_dfog - eMaj_dfog * eBot.dx);
      }
      span.interpMask |= 0x001;
      if (ctx->Light.ShadeModel == 0x1D01) {
         GLfloat eMaj_dr, eBot_dr;
         GLfloat eMaj_dg, eBot_dg;
         GLfloat eMaj_db, eBot_db;
         GLfloat eMaj_da, eBot_da;
         eMaj_dr = (GLfloat) ((GLint) vMax->color[0] -
                             (GLint) vMin->color[0]);
         eBot_dr = (GLfloat) ((GLint) vMid->color[0] -
                             (GLint) vMin->color[0]);
         drdx = oneOverArea * (eMaj_dr * eBot.dy - eMaj.dy * eBot_dr);
         span.redStep = (((int) ((((drdx) * 2048.0f) >= 0.0F) ? (((drdx) * 2048.0f) + 0.5F) : (((drdx) * 2048.0f) - 0.5F))));
         drdy = oneOverArea * (eMaj.dx * eBot_dr - eMaj_dr * eBot.dx);
         eMaj_dg = (GLfloat) ((GLint) vMax->color[1] -
                             (GLint) vMin->color[1]);
         eBot_dg = (GLfloat) ((GLint) vMid->color[1] -
                             (GLint) vMin->color[1]);
         dgdx = oneOverArea * (eMaj_dg * eBot.dy - eMaj.dy * eBot_dg);
         span.greenStep = (((int) ((((dgdx) * 2048.0f) >= 0.0F) ? (((dgdx) * 2048.0f) + 0.5F) : (((dgdx) * 2048.0f) - 0.5F))));
         dgdy = oneOverArea * (eMaj.dx * eBot_dg - eMaj_dg * eBot.dx);
         eMaj_db = (GLfloat) ((GLint) vMax->color[2] -
                             (GLint) vMin->color[2]);
         eBot_db = (GLfloat) ((GLint) vMid->color[2] -
                             (GLint) vMin->color[2]);
         dbdx = oneOverArea * (eMaj_db * eBot.dy - eMaj.dy * eBot_db);
         span.blueStep = (((int) ((((dbdx) * 2048.0f) >= 0.0F) ? (((dbdx) * 2048.0f) + 0.5F) : (((dbdx) * 2048.0f) - 0.5F))));
         dbdy = oneOverArea * (eMaj.dx * eBot_db - eMaj_db * eBot.dx);
         eMaj_da = (GLfloat) ((GLint) vMax->color[3] -
                             (GLint) vMin->color[3]);
         eBot_da = (GLfloat) ((GLint) vMid->color[3] -
                             (GLint) vMin->color[3]);
         dadx = oneOverArea * (eMaj_da * eBot.dy - eMaj.dy * eBot_da);
         span.alphaStep = (((int) ((((dadx) * 2048.0f) >= 0.0F) ? (((dadx) * 2048.0f) + 0.5F) : (((dadx) * 2048.0f) - 0.5F))));
         dady = oneOverArea * (eMaj.dx * eBot_da - eMaj_da * eBot.dx);
      }
      else {
         ;
         span.interpMask |= 0x200;
         drdx = drdy = 0.0F;
         dgdx = dgdy = 0.0F;
         dbdx = dbdy = 0.0F;
         span.redStep = 0;
         span.greenStep = 0;
         span.blueStep = 0;
         dadx = dady = 0.0F;
         span.alphaStep = 0;
      }
      span.interpMask |= 0x002;
      if (ctx->Light.ShadeModel == 0x1D01) {
         GLfloat eMaj_dsr, eBot_dsr;
         GLfloat eMaj_dsg, eBot_dsg;
         GLfloat eMaj_dsb, eBot_dsb;
         eMaj_dsr = (GLfloat) ((GLint) vMax->specular[0] -
                              (GLint) vMin->specular[0]);
         eBot_dsr = (GLfloat) ((GLint) vMid->specular[0] -
                              (GLint) vMin->specular[0]);
         dsrdx = oneOverArea * (eMaj_dsr * eBot.dy - eMaj.dy * eBot_dsr);
         span.specRedStep = (((int) ((((dsrdx) * 2048.0f) >= 0.0F) ? (((dsrdx) * 2048.0f) + 0.5F) : (((dsrdx) * 2048.0f) - 0.5F))));
         dsrdy = oneOverArea * (eMaj.dx * eBot_dsr - eMaj_dsr * eBot.dx);
         eMaj_dsg = (GLfloat) ((GLint) vMax->specular[1] -
                              (GLint) vMin->specular[1]);
         eBot_dsg = (GLfloat) ((GLint) vMid->specular[1] -
                              (GLint) vMin->specular[1]);
         dsgdx = oneOverArea * (eMaj_dsg * eBot.dy - eMaj.dy * eBot_dsg);
         span.specGreenStep = (((int) ((((dsgdx) * 2048.0f) >= 0.0F) ? (((dsgdx) * 2048.0f) + 0.5F) : (((dsgdx) * 2048.0f) - 0.5F))));
         dsgdy = oneOverArea * (eMaj.dx * eBot_dsg - eMaj_dsg * eBot.dx);
         eMaj_dsb = (GLfloat) ((GLint) vMax->specular[2] -
                              (GLint) vMin->specular[2]);
         eBot_dsb = (GLfloat) ((GLint) vMid->specular[2] -
                              (GLint) vMin->specular[2]);
         dsbdx = oneOverArea * (eMaj_dsb * eBot.dy - eMaj.dy * eBot_dsb);
         span.specBlueStep = (((int) ((((dsbdx) * 2048.0f) >= 0.0F) ? (((dsbdx) * 2048.0f) + 0.5F) : (((dsbdx) * 2048.0f) - 0.5F))));
         dsbdy = oneOverArea * (eMaj.dx * eBot_dsb - eMaj_dsb * eBot.dx);
      }
      else {
         dsrdx = dsrdy = 0.0F;
         dsgdx = dsgdy = 0.0F;
         dsbdx = dsbdy = 0.0F;
         span.specRedStep = 0;
         span.specGreenStep = 0;
         span.specBlueStep = 0;
      }
      span.interpMask |= 0x020;
      {
         GLfloat wMax = vMax->win[3];
         GLfloat wMin = vMin->win[3];
         GLfloat wMid = vMid->win[3];
         GLuint u;
         for (u = 0; u < ctx->Const.MaxTextureUnits; u++) {
            if (ctx->Texture.Unit[u]._ReallyEnabled) {
               GLfloat eMaj_ds, eBot_ds;
               GLfloat eMaj_dt, eBot_dt;
               GLfloat eMaj_du, eBot_du;
               GLfloat eMaj_dv, eBot_dv;
               eMaj_ds = vMax->texcoord[u][0] * wMax
                       - vMin->texcoord[u][0] * wMin;
               eBot_ds = vMid->texcoord[u][0] * wMid
                       - vMin->texcoord[u][0] * wMin;
               dsdx[u] = oneOverArea * (eMaj_ds * eBot.dy - eMaj.dy * eBot_ds);
               dsdy[u] = oneOverArea * (eMaj.dx * eBot_ds - eMaj_ds * eBot.dx);
               span.texStepX[u][0] = dsdx[u];
               span.texStepY[u][0] = dsdy[u];

               eMaj_dt = vMax->texcoord[u][1] * wMax
                       - vMin->texcoord[u][1] * wMin;
               eBot_dt = vMid->texcoord[u][1] * wMid
                       - vMin->texcoord[u][1] * wMin;
               dtdx[u] = oneOverArea * (eMaj_dt * eBot.dy - eMaj.dy * eBot_dt);
               dtdy[u] = oneOverArea * (eMaj.dx * eBot_dt - eMaj_dt * eBot.dx);
               span.texStepX[u][1] = dtdx[u];
               span.texStepY[u][1] = dtdy[u];

               eMaj_du = vMax->texcoord[u][2] * wMax
                       - vMin->texcoord[u][2] * wMin;
               eBot_du = vMid->texcoord[u][2] * wMid
                       - vMin->texcoord[u][2] * wMin;
               dudx[u] = oneOverArea * (eMaj_du * eBot.dy - eMaj.dy * eBot_du);
               dudy[u] = oneOverArea * (eMaj.dx * eBot_du - eMaj_du * eBot.dx);
               span.texStepX[u][2] = dudx[u];
               span.texStepY[u][2] = dudy[u];

               eMaj_dv = vMax->texcoord[u][3] * wMax
                       - vMin->texcoord[u][3] * wMin;
               eBot_dv = vMid->texcoord[u][3] * wMid
                       - vMin->texcoord[u][3] * wMin;
               dvdx[u] = oneOverArea * (eMaj_dv * eBot.dy - eMaj.dy * eBot_dv);
               dvdy[u] = oneOverArea * (eMaj.dx * eBot_dv - eMaj_dv * eBot.dx);
               span.texStepX[u][3] = dvdx[u];
               span.texStepY[u][3] = dvdy[u];
            }
         }
      }

      /*
       * We always sample at pixel centers.  However, we avoid
       * explicit half-pixel offsets in this code by incorporating
       * the proper offset in each of x and y during the
       * transformation to window coordinates.
       *
       * We also apply the usual rasterization rules to prevent
       * cracks and overlaps.  A pixel is considered inside a
       * subtriangle if it meets all of four conditions: it is on or
       * to the right of the left edge, strictly to the left of the
       * right edge, on or below the top edge, and strictly above
       * the bottom edge.  (Some edges may be degenerate.)
       *
       * The following discussion assumes left-to-right scanning
       * (that is, the major edge is on the left); the right-to-left
       * case is a straightforward variation.
       *
       * We start by finding the half-integral y coordinate that is
       * at or below the top of the triangle.  This gives us the
       * first scan line that could possibly contain pixels that are
       * inside the triangle.
       *
       * Next we creep down the major edge until we reach that y,
       * and compute the corresponding x coordinate on the edge.
       * Then we find the half-integral x that lies on or just
       * inside the edge.  This is the first pixel that might lie in
       * the interior of the triangle.  (We won't know for sure
       * until we check the other edges.)
       *
       * As we rasterize the triangle, we'll step down the major
       * edge.  For each step in y, we'll move an integer number
       * of steps in x.  There are two possible x step sizes, which
       * we'll call the ``inner'' step (guaranteed to land on the
       * edge or inside it) and the ``outer'' step (guaranteed to
       * land on the edge or outside it).  The inner and outer steps
       * differ by one.  During rasterization we maintain an error
       * term that indicates our distance from the true edge, and
       * select either the inner step or the outer step, whichever
       * gets us to the first pixel that falls inside the triangle.
       *
       * All parameters (z, red, etc.) as well as the buffer
       * addresses for color and z have inner and outer step values,
       * so that we can increment them appropriately.  This method
       * eliminates the need to adjust parameters by creeping a
       * sub-pixel amount into the triangle at each scanline.
       */

      {
         int subTriangle;
         GLfixed fx;
         GLfixed fxLeftEdge = 0, fxRightEdge = 0;
         GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
         GLfixed fdxOuter;
         int idxOuter;
         float dxOuter;
         GLfixed fError = 0, fdError = 0;
         float adjx, adjy;
         GLfixed fy;
         GLushort *zRow = 0;
         int dZRowOuter = 0, dZRowInner;  /* offset in bytes */
         GLfixed fz = 0, fdzOuter = 0, fdzInner;
         GLfloat fogLeft = 0, dfogOuter = 0, dfogInner;
         GLfixed fr = 0, fdrOuter = 0, fdrInner;
         GLfixed fg = 0, fdgOuter = 0, fdgInner;
         GLfixed fb = 0, fdbOuter = 0, fdbInner;
         GLfixed fa = 0, fdaOuter = 0, fdaInner;
         GLfixed fsr=0, fdsrOuter=0, fdsrInner;
         GLfixed fsg=0, fdsgOuter=0, fdsgInner;
         GLfixed fsb=0, fdsbOuter=0, fdsbInner;
         GLfloat sLeft[8];
         GLfloat tLeft[8];
         GLfloat uLeft[8];
         GLfloat vLeft[8];
         GLfloat dsOuter[8], dsInner[8];
         GLfloat dtOuter[8], dtInner[8];
         GLfloat duOuter[8], duInner[8];
         GLfloat dvOuter[8], dvInner[8];

         for (subTriangle=0; subTriangle<=1; subTriangle++) {
            EdgeT *eLeft, *eRight;
            int setupLeft, setupRight;
            int lines;

            if (subTriangle==0) {
               /* bottom half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eBot;
                  lines = eRight->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
               else {
                  eLeft = &eBot;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
            }
            else {
               /* top half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eTop;
                  lines = eRight->lines;
                  setupLeft = 0;
                  setupRight = 1;
               }
               else {
                  eLeft = &eTop;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 0;
               }
               if (lines == 0)
                  return;
            }

            if (setupLeft && eLeft->lines > 0) {
               const SWvertex *vLower;
               GLfixed fsx = eLeft->fsx;
               fx = (((fsx) + 0x00000800 - 1) & (~0x000007FF));
               fError = fx - fsx - 0x00000800;
               fxLeftEdge = fsx - 1;
               fdxLeftEdge = eLeft->fdxdy;
               fdxOuter = ((fdxLeftEdge - 1) & (~0x000007FF));
               fdError = fdxOuter - fdxLeftEdge + 0x00000800;
               idxOuter = ((fdxOuter) >> 11);
               dxOuter = (float) idxOuter;
               (void) dxOuter;

               fy = eLeft->fsy;
               span.y = ((fy) >> 11);

               adjx = (float)(fx - eLeft->fx0);  /* SCALED! */
               adjy = eLeft->adjy;              /* SCALED! */
               vLower = eLeft->v0;

               /*
                * Now we need the set of parameter (z, color, etc.) values at
                * the point (fx, fy).  This gives us properly-sampled parameter
                * values that we can step from pixel to pixel.  Furthermore,
                * although we might have intermediate results that overflow
                * the normal parameter range when we step temporarily outside
                * the triangle, we shouldn't overflow or underflow for any
                * pixel that's actually inside the triangle.
                */

               {
                  GLfloat z0 = vLower->win[2];
                  if (depthBits <= 16) {
                     /* interpolate fixed-pt values */
                     GLfloat tmp = (z0 * 2048.0f +
                                    dzdx * adjx + dzdy * adjy) + 0x00000400;
                     if (tmp < 0xffffffff / 2)
                        fz = (GLfixed) tmp;
                     else
                        fz = 0xffffffff / 2;
                     fdzOuter = (((int) ((((dzdy + dxOuter * dzdx) * 2048.0f) >= 0.0F) ? (((dzdy + dxOuter * dzdx) * 2048.0f) + 0.5F) : (((dzdy + dxOuter * dzdx) * 2048.0f) - 0.5F))));
                  }
                  else {
                     /* interpolate depth values exactly */
                     fz = (GLint) (z0 + dzdx * ((adjx) * (1.0F / 2048.0f))
                                   + dzdy * ((adjy) * (1.0F / 2048.0f)));
                     fdzOuter = (GLint) (dzdy + dxOuter * dzdx);
                  }
                  zRow = (GLushort *)
                    _mesa_zbuffer_address(ctx, ((fxLeftEdge) >> 11), span.y);
                  dZRowOuter = (ctx->DrawBuffer->Width + idxOuter) * sizeof(GLushort);
               }
               fogLeft = vLower->fog + (span.fogStep * adjx + dfogdy * adjy)
                                       * (1.0F/2048.0f);
               dfogOuter = dfogdy + dxOuter * span.fogStep;
               if (ctx->Light.ShadeModel == 0x1D01) {
                  fr = (GLfixed) (((vLower->color[0]) << 11)
                                   + drdx * adjx + drdy * adjy) + 0x00000400;
                  fdrOuter = (((int) ((((drdy + dxOuter * drdx) * 2048.0f) >= 0.0F) ? (((drdy + dxOuter * drdx) * 2048.0f) + 0.5F) : (((drdy + dxOuter * drdx) * 2048.0f) - 0.5F))));
                  fg = (GLfixed) (((vLower->color[1]) << 11)
                                   + dgdx * adjx + dgdy * adjy) + 0x00000400;
                  fdgOuter = (((int) ((((dgdy + dxOuter * dgdx) * 2048.0f) >= 0.0F) ? (((dgdy + dxOuter * dgdx) * 2048.0f) + 0.5F) : (((dgdy + dxOuter * dgdx) * 2048.0f) - 0.5F))));
                  fb = (GLfixed) (((vLower->color[2]) << 11)
                                    + dbdx * adjx + dbdy * adjy) + 0x00000400;
                  fdbOuter = (((int) ((((dbdy + dxOuter * dbdx) * 2048.0f) >= 0.0F) ? (((dbdy + dxOuter * dbdx) * 2048.0f) + 0.5F) : (((dbdy + dxOuter * dbdx) * 2048.0f) - 0.5F))));
                  fa = (GLfixed) (((vLower->color[3]) << 11)
                                   + dadx * adjx + dady * adjy) + 0x00000400;
                  fdaOuter = (((int) ((((dady + dxOuter * dadx) * 2048.0f) >= 0.0F) ? (((dady + dxOuter * dadx) * 2048.0f) + 0.5F) : (((dady + dxOuter * dadx) * 2048.0f) - 0.5F))));
               }
               else {
                  ;
                  fr = ((v2->color[0]) << 11);
                  fg = ((v2->color[1]) << 11);
                  fb = ((v2->color[2]) << 11);
                  fdrOuter = fdgOuter = fdbOuter = 0;
                  fa =  ((v2->color[3]) << 11);
                  fdaOuter = 0;
               }
               if (ctx->Light.ShadeModel == 0x1D01) {
                  fsr = (GLfixed) (((vLower->specular[0]) << 11)
                                   + dsrdx * adjx + dsrdy * adjy) + 0x00000400;
                  fdsrOuter = (((int) ((((dsrdy + dxOuter * dsrdx) * 2048.0f) >= 0.0F) ? (((dsrdy + dxOuter * dsrdx) * 2048.0f) + 0.5F) : (((dsrdy + dxOuter * dsrdx) * 2048.0f) - 0.5F))));
                  fsg = (GLfixed) (((vLower->specular[1]) << 11)
                                   + dsgdx * adjx + dsgdy * adjy) + 0x00000400;
                  fdsgOuter = (((int) ((((dsgdy + dxOuter * dsgdx) * 2048.0f) >= 0.0F) ? (((dsgdy + dxOuter * dsgdx) * 2048.0f) + 0.5F) : (((dsgdy + dxOuter * dsgdx) * 2048.0f) - 0.5F))));
                  fsb = (GLfixed) (((vLower->specular[2]) << 11)
                                   + dsbdx * adjx + dsbdy * adjy) + 0x00000400;
                  fdsbOuter = (((int) ((((dsbdy + dxOuter * dsbdx) * 2048.0f) >= 0.0F) ? (((dsbdy + dxOuter * dsbdx) * 2048.0f) + 0.5F) : (((dsbdy + dxOuter * dsbdx) * 2048.0f) - 0.5F))));
               }
               else {
                  fsr = ((v2->specular[0]) << 11);
                  fsg = ((v2->specular[1]) << 11);
                  fsb = ((v2->specular[2]) << 11);
                  fdsrOuter = fdsgOuter = fdsbOuter = 0;
               }
               {
                  GLuint u;
                  for (u = 0; u < ctx->Const.MaxTextureUnits; u++) {
                     if (ctx->Texture.Unit[u]._ReallyEnabled) {
                        GLfloat invW = vLower->win[3];
                        GLfloat s0, t0, u0, v0;
                        s0 = vLower->texcoord[u][0] * invW;
                        sLeft[u] = s0 + (span.texStepX[u][0] * adjx + dsdy[u]
                                         * adjy) * (1.0F/2048.0f);
                        dsOuter[u] = dsdy[u] + dxOuter * span.texStepX[u][0];
                        t0 = vLower->texcoord[u][1] * invW;
                        tLeft[u] = t0 + (span.texStepX[u][1] * adjx + dtdy[u]
                                         * adjy) * (1.0F/2048.0f);
                        dtOuter[u] = dtdy[u] + dxOuter * span.texStepX[u][1];
                        u0 = vLower->texcoord[u][2] * invW;
                        uLeft[u] = u0 + (span.texStepX[u][2] * adjx + dudy[u]
                                         * adjy) * (1.0F/2048.0f);
                        duOuter[u] = dudy[u] + dxOuter * span.texStepX[u][2];
                        v0 = vLower->texcoord[u][3] * invW;
                        vLeft[u] = v0 + (span.texStepX[u][3] * adjx + dvdy[u]
                                         * adjy) * (1.0F/2048.0f);
                        dvOuter[u] = dvdy[u] + dxOuter * span.texStepX[u][3];
                     }
                  }
               }

            } /*if setupLeft*/


            if (setupRight && eRight->lines>0) {
               fxRightEdge = eRight->fsx - 1;
               fdxRightEdge = eRight->fdxdy;
            }

            if (lines==0) {
               continue;
            }


            /* Rasterize setup */
            dZRowInner = dZRowOuter + sizeof(GLushort);
            fdzInner = fdzOuter + span.zStep;
            dfogInner = dfogOuter + span.fogStep;
            fdrInner = fdrOuter + span.redStep;
            fdgInner = fdgOuter + span.greenStep;
            fdbInner = fdbOuter + span.blueStep;
            fdaInner = fdaOuter + span.alphaStep;
            fdsrInner = fdsrOuter + span.specRedStep;
            fdsgInner = fdsgOuter + span.specGreenStep;
            fdsbInner = fdsbOuter + span.specBlueStep;
            {
               GLuint u;
               for (u = 0; u < ctx->Const.MaxTextureUnits; u++) {
                  if (ctx->Texture.Unit[u]._ReallyEnabled) {
                     dsInner[u] = dsOuter[u] + span.texStepX[u][0];
                     dtInner[u] = dtOuter[u] + span.texStepX[u][1];
                     duInner[u] = duOuter[u] + span.texStepX[u][2];
                     dvInner[u] = dvOuter[u] + span.texStepX[u][3];
                  }
               }
            }

            while (lines > 0) {
               /* initialize the span interpolants to the leftmost value */
               /* ff = fixed-pt fragment */
               const GLint right = ((fxRightEdge) >> 11);

               span.x = ((fxLeftEdge) >> 11);

               if (right <= span.x)
                  span.end = 0;
               else
                  span.end = right - span.x;

               span.z = fz;
               span.fog = fogLeft;
               span.red = fr;
               span.green = fg;
               span.blue = fb;
               span.alpha = fa;
               span.specRed = fsr;
               span.specGreen = fsg;
               span.specBlue = fsb;


               {
                  GLuint u;
                  for (u = 0; u < ctx->Const.MaxTextureUnits; u++) {
                     if (ctx->Texture.Unit[u]._ReallyEnabled) {
                        span.tex[u][0] = sLeft[u];
                        span.tex[u][1] = tLeft[u];
                        span.tex[u][2] = uLeft[u];
                        span.tex[u][3] = vLeft[u];
                     }
                  }
               }

               {
                  /* need this to accomodate round-off errors */
                  const GLint len = right - span.x - 1;
                  GLfixed ffrend = span.red + len * span.redStep;
                  GLfixed ffgend = span.green + len * span.greenStep;
                  GLfixed ffbend = span.blue + len * span.blueStep;
                  if (ffrend < 0) {
                     span.red -= ffrend;
                     if (span.red < 0)
                        span.red = 0;
                  }
                  if (ffgend < 0) {
                     span.green -= ffgend;
                     if (span.green < 0)
                        span.green = 0;
                  }
                  if (ffbend < 0) {
                     span.blue -= ffbend;
                     if (span.blue < 0)
                        span.blue = 0;
                  }
               }
               {
                  const GLint len = right - span.x - 1;
                  GLfixed ffaend = span.alpha + len * span.alphaStep;
                  if (ffaend < 0) {
                     span.alpha -= ffaend;
                     if (span.alpha < 0)
                        span.alpha = 0;
                  }
               }
               {
                  /* need this to accomodate round-off errors */
                  const GLint len = right - span.x - 1;
                  GLfixed ffsrend = span.specRed + len * span.specRedStep;
                  GLfixed ffsgend = span.specGreen + len * span.specGreenStep;
                  GLfixed ffsbend = span.specBlue + len * span.specBlueStep;
                  if (ffsrend < 0) {
                     span.specRed -= ffsrend;
                     if (span.specRed < 0)
                        span.specRed = 0;
                  }
                  if (ffsgend < 0) {
                     span.specGreen -= ffsgend;
                     if (span.specGreen < 0)
                        span.specGreen = 0;
                  }
                  if (ffsbend < 0) {
                     span.specBlue -= ffsbend;
                     if (span.specBlue < 0)
                        span.specBlue = 0;
                  }
               }

               /* This is where we actually generate fragments */
               if (span.end > 0) {
                  _mesa_write_texture_span(ctx, &span);;
               }

               /*
                * Advance to the next scan line.  Compute the
                * new edge coordinates, and adjust the
                * pixel-center x coordinate so that it stays
                * on or inside the major edge.
                */
               (span.y)++;
               lines--;

               fxLeftEdge += fdxLeftEdge;
               fxRightEdge += fdxRightEdge;


               fError += fdError;
               if (fError >= 0) {
                  fError -= 0x00000800;
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowOuter);
                  fz += fdzOuter;
                  fogLeft += dfogOuter;
                  fr += fdrOuter;
                  fg += fdgOuter;
                  fb += fdbOuter;
                  fa += fdaOuter;
                  fsr += fdsrOuter;
                  fsg += fdsgOuter;
                  fsb += fdsbOuter;
                  {
                     GLuint u;
                     for (u = 0; u < ctx->Const.MaxTextureUnits; u++) {
                        if (ctx->Texture.Unit[u]._ReallyEnabled) {
                           sLeft[u] += dsOuter[u];
                           tLeft[u] += dtOuter[u];
                           uLeft[u] += duOuter[u];
                           vLeft[u] += dvOuter[u];
                        }
                     }
                  }
               }
               else {
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowInner);
                  fz += fdzInner;
                  fogLeft += dfogInner;
                  fr += fdrInner;
                  fg += fdgInner;
                  fb += fdbInner;
                  fa += fdaInner;
                  fsr += fdsrInner;
                  fsg += fdsgInner;
                  fsb += fdsbInner;
                  {
                     GLuint u;
                     for (u = 0; u < ctx->Const.MaxTextureUnits; u++) {
                        if (ctx->Texture.Unit[u]._ReallyEnabled) {
                           sLeft[u] += dsInner[u];
                           tLeft[u] += dtInner[u];
                           uLeft[u] += duInner[u];
                           vLeft[u] += dvInner[u];
                        }
                     }
                  }
               }
            } /*while lines>0*/

         } /* for subTriangle */

      }
   }
}


/*
 * Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom triangle rasterizers.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be interplated across the triangle:
 *    INTERP_Z        - if defined, interpolate Z values
 *    INTERP_FOG      - if defined, interpolate fog values
 *    INTERP_RGB      - if defined, interpolate RGB values
 *    INTERP_ALPHA    - if defined, interpolate Alpha values (req's INTERP_RGB)
 *    INTERP_SPEC     - if defined, interpolate specular RGB values
 *    INTERP_INDEX    - if defined, interpolate color index values
 *    INTERP_INT_TEX  - if defined, interpolate integer ST texcoords
 *                         (fast, simple 2-D texture mapping)
 *    INTERP_TEX      - if defined, interpolate set 0 float STRQ texcoords
 *                         NOTE:  OpenGL STRQ = Mesa STUV (R was taken for red)
 *    INTERP_MULTITEX - if defined, interpolate N units of STRQ texcoords
 *    INTERP_FLOAT_RGBA - if defined, interpolate RGBA with floating point
 *    INTERP_FLOAT_SPEC - if defined, interpolate specular with floating point
 *
 * When one can directly address pixels in the color buffer the following
 * macros can be defined and used to compute pixel addresses during
 * rasterization (see pRow):
 *    PIXEL_TYPE          - the datatype of a pixel (GLubyte, GLushort, GLuint)
 *    BYTES_PER_ROW       - number of bytes per row in the color buffer
 *    PIXEL_ADDRESS(X,Y)  - returns the address of pixel at (X,Y) where
 *                          Y==0 at bottom of screen and increases upward.
 *
 * Similarly, for direct depth buffer access, this type is used for depth
 * buffer addressing:
 *    DEPTH_TYPE          - either GLushort or GLuint
 *
 * Optionally, one may provide one-time setup code per triangle:
 *    SETUP_CODE    - code which is to be executed once per triangle
 *    CLEANUP_CODE    - code to execute at end of triangle
 *
 * The following macro MUST be defined:
 *    RENDER_SPAN(span) - code to write a span of pixels.
 *
 * This code was designed for the origin to be in the lower-left corner.
 *
 * Inspired by triangle rasterizer code written by Allen Akin.  Thanks Allen!
 */


/*
 * This is a bit of a hack, but it's a centralized place to enable floating-
 * point color interpolation when GLchan is actually floating point.
 */



static void occlusion_zless_triangle(GLcontext *ctx, const SWvertex *v0,
                                 const SWvertex *v1,
                                 const SWvertex *v2 )
{
   typedef struct {
        const SWvertex *v0, *v1;   /* Y(v0) < Y(v1) */
        GLfloat dx;    /* X(v1) - X(v0) */
        GLfloat dy;    /* Y(v1) - Y(v0) */
        GLfixed fdxdy; /* dx/dy in fixed-point */
        GLfixed fsx;   /* first sample point x coord */
        GLfixed fsy;
        GLfloat adjy;  /* adjust from v[0]->fy to fsy, scaled */
        GLint lines;   /* number of lines to be sampled on this edge */
        GLfixed fx0;   /* fixed pt X of lower endpoint */
   } EdgeT;

   const GLint depthBits = ctx->Visual.depthBits;
   const GLint fixedToDepthShift = depthBits <= 16 ? FIXED_SHIFT : 0;
   const GLfloat maxDepth = ctx->DepthMaxF;
   EdgeT eMaj, eTop, eBot;
   GLfloat oneOverArea;
   const SWvertex *vMin, *vMid, *vMax;  /* Y(vMin)<=Y(vMid)<=Y(vMax) */
   float bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;
   const GLint snapMask = ~((0x00000800 / (1 << 4)) - 1); /* for x/y coord snapping */
   GLfixed vMin_fx, vMin_fy, vMid_fx, vMid_fy, vMax_fx, vMax_fy;

   struct sw_span span;

   do { (span).primitive = (0x0009); (span).interpMask = (0); (span).arrayMask = (0); (span).start = 0; (span).end = (0); (span).facing = 0; (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays; } while (0);


   printf("%s()\n", __FUNCTION__);
   /*
   printf("  %g, %g, %g\n", v0->win[0], v0->win[1], v0->win[2]);
   printf("  %g, %g, %g\n", v1->win[0], v1->win[1], v1->win[2]);
   printf("  %g, %g, %g\n", v2->win[0], v2->win[1], v2->win[2]);
    */

   /* Compute fixed point x,y coords w/ half-pixel offsets and snapping.
    * And find the order of the 3 vertices along the Y axis.
    */
   {
      const GLfixed fy0 = (((int) ((((v0->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v0->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v0->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy1 = (((int) ((((v1->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v1->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v1->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      const GLfixed fy2 = (((int) ((((v2->win[1] - 0.5F) * 2048.0f) >= 0.0F) ? (((v2->win[1] - 0.5F) * 2048.0f) + 0.5F) : (((v2->win[1] - 0.5F) * 2048.0f) - 0.5F)))) & snapMask;

      if (fy0 <= fy1) {
         if (fy1 <= fy2) {
            /* y0 <= y1 <= y2 */
            vMin = v0;   vMid = v1;   vMax = v2;
            vMin_fy = fy0;  vMid_fy = fy1;  vMax_fy = fy2;
         }
         else if (fy2 <= fy0) {
            /* y2 <= y0 <= y1 */
            vMin = v2;   vMid = v0;   vMax = v1;
            vMin_fy = fy2;  vMid_fy = fy0;  vMax_fy = fy1;
         }
         else {
            /* y0 <= y2 <= y1 */
            vMin = v0;   vMid = v2;   vMax = v1;
            vMin_fy = fy0;  vMid_fy = fy2;  vMax_fy = fy1;
            bf = -bf;
         }
      }
      else {
         if (fy0 <= fy2) {
            /* y1 <= y0 <= y2 */
            vMin = v1;   vMid = v0;   vMax = v2;
            vMin_fy = fy1;  vMid_fy = fy0;  vMax_fy = fy2;
            bf = -bf;
         }
         else if (fy2 <= fy1) {
            /* y2 <= y1 <= y0 */
            vMin = v2;   vMid = v1;   vMax = v0;
            vMin_fy = fy2;  vMid_fy = fy1;  vMax_fy = fy0;
            bf = -bf;
         }
         else {
            /* y1 <= y2 <= y0 */
            vMin = v1;   vMid = v2;   vMax = v0;
            vMin_fy = fy1;  vMid_fy = fy2;  vMax_fy = fy0;
         }
      }

      /* fixed point X coords */
      vMin_fx = (((int) ((((vMin->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMin->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMin->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMid_fx = (((int) ((((vMid->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMid->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMid->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
      vMax_fx = (((int) ((((vMax->win[0] + 0.5F) * 2048.0f) >= 0.0F) ? (((vMax->win[0] + 0.5F) * 2048.0f) + 0.5F) : (((vMax->win[0] + 0.5F) * 2048.0f) - 0.5F)))) & snapMask;
   }

   /* vertex/edge relationship */
   eMaj.v0 = vMin;   eMaj.v1 = vMax;   /*TODO: .v1's not needed */
   eTop.v0 = vMid;   eTop.v1 = vMax;
   eBot.v0 = vMin;   eBot.v1 = vMid;

   /* compute deltas for each edge:  vertex[upper] - vertex[lower] */
   eMaj.dx = ((vMax_fx - vMin_fx) * (1.0F / 2048.0f));
   eMaj.dy = ((vMax_fy - vMin_fy) * (1.0F / 2048.0f));
   eTop.dx = ((vMax_fx - vMid_fx) * (1.0F / 2048.0f));
   eTop.dy = ((vMax_fy - vMid_fy) * (1.0F / 2048.0f));
   eBot.dx = ((vMid_fx - vMin_fx) * (1.0F / 2048.0f));
   eBot.dy = ((vMid_fy - vMin_fy) * (1.0F / 2048.0f));

   /* compute area, oneOverArea and perform backface culling */
   {
      const GLfloat area = eMaj.dx * eBot.dy - eBot.dx * eMaj.dy;

      /* Do backface culling */
      if (area * bf < 0.0)
         return;

      if (IS_INF_OR_NAN(area) || area == 0.0F)
         return;

      oneOverArea = 1.0F / area;
   }

   span.facing = ctx->_Facing; /* for 2-sided stencil test */

   /* Edge setup.  For a triangle strip these could be reused... */
   {
      eMaj.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eMaj.lines = (((((vMax_fy - eMaj.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eMaj.lines > 0) {
         GLfloat dxdy = eMaj.dx / eMaj.dy;
         eMaj.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eMaj.adjy = (GLfloat) (eMaj.fsy - vMin_fy);  /* SCALED! */
         eMaj.fx0 = vMin_fx;
         eMaj.fsx = eMaj.fx0 + (GLfixed) (eMaj.adjy * dxdy);
      }
      else {
         return;  /*CULLED*/
      }

      eTop.fsy = (((vMid_fy) + 0x00000800 - 1) & (~0x000007FF));
      eTop.lines = (((((vMax_fy - eTop.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eTop.lines > 0) {
         GLfloat dxdy = eTop.dx / eTop.dy;
         eTop.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eTop.adjy = (GLfloat) (eTop.fsy - vMid_fy); /* SCALED! */
         eTop.fx0 = vMid_fx;
         eTop.fsx = eTop.fx0 + (GLfixed) (eTop.adjy * dxdy);
      }

      eBot.fsy = (((vMin_fy) + 0x00000800 - 1) & (~0x000007FF));
      eBot.lines = (((((vMid_fy - eBot.fsy) + 0x00000800 - 1) & (~0x000007FF))) >> 11);
      if (eBot.lines > 0) {
         GLfloat dxdy = eBot.dx / eBot.dy;
         eBot.fdxdy = (((int) ((((dxdy) * 2048.0f) >= 0.0F) ? (((dxdy) * 2048.0f) + 0.5F) : (((dxdy) * 2048.0f) - 0.5F))));
         eBot.adjy = (GLfloat) (eBot.fsy - vMin_fy);  /* SCALED! */
         eBot.fx0 = vMin_fx;
         eBot.fsx = eBot.fx0 + (GLfixed) (eBot.adjy * dxdy);
      }
   }

   /*
    * Conceptually, we view a triangle as two subtriangles
    * separated by a perfectly horizontal line.  The edge that is
    * intersected by this line is one with maximal absolute dy; we
    * call it a ``major'' edge.  The other two edges are the
    * ``top'' edge (for the upper subtriangle) and the ``bottom''
    * edge (for the lower subtriangle).  If either of these two
    * edges is horizontal or very close to horizontal, the
    * corresponding subtriangle might cover zero sample points;
    * we take care to handle such cases, for performance as well
    * as correctness.
    *
    * By stepping rasterization parameters along the major edge,
    * we can avoid recomputing them at the discontinuity where
    * the top and bottom edges meet.  However, this forces us to
    * be able to scan both left-to-right and right-to-left.
    * Also, we must determine whether the major edge is at the
    * left or right side of the triangle.  We do this by
    * computing the magnitude of the cross-product of the major
    * and top edges.  Since this magnitude depends on the sine of
    * the angle between the two edges, its sign tells us whether
    * we turn to the left or to the right when travelling along
    * the major edge to the top edge, and from this we infer
    * whether the major edge is on the left or the right.
    *
    * Serendipitously, this cross-product magnitude is also a
    * value we need to compute the iteration parameter
    * derivatives for the triangle, and it can be used to perform
    * backface culling because its sign tells us whether the
    * triangle is clockwise or counterclockwise.  In this code we
    * refer to it as ``area'' because it's also proportional to
    * the pixel area of the triangle.
    */

   {
      GLint scan_from_left_to_right;  /* true if scanning left-to-right */
      GLfloat dzdx, dzdy;

      /*
       * Execute user-supplied setup code
       */
      if (ctx->OcclusionResult) { return; }

      scan_from_left_to_right = (oneOverArea < 0.0F);


      /* compute d?/dx and d?/dy derivatives */
      span.interpMask |= 0x008;
      {
         GLfloat eMaj_dz, eBot_dz;
         eMaj_dz = vMax->win[2] - vMin->win[2];
         eBot_dz = vMid->win[2] - vMin->win[2];
         dzdx = oneOverArea * (eMaj_dz * eBot.dy - eMaj.dy * eBot_dz);
         if (dzdx > maxDepth || dzdx < -maxDepth) {
            /* probably a sliver triangle */
            dzdx = 0.0;
            dzdy = 0.0;
         }
         else {
            dzdy = oneOverArea * (eMaj.dx * eBot_dz - eMaj_dz * eBot.dx);
         }
         if (depthBits <= 16)
            span.zStep = (((int) ((((dzdx) * 2048.0f) >= 0.0F) ? (((dzdx) * 2048.0f) + 0.5F) : (((dzdx) * 2048.0f) - 0.5F))));
         else
            span.zStep = (GLint) dzdx;
      }

      /*
       * We always sample at pixel centers.  However, we avoid
       * explicit half-pixel offsets in this code by incorporating
       * the proper offset in each of x and y during the
       * transformation to window coordinates.
       *
       * We also apply the usual rasterization rules to prevent
       * cracks and overlaps.  A pixel is considered inside a
       * subtriangle if it meets all of four conditions: it is on or
       * to the right of the left edge, strictly to the left of the
       * right edge, on or below the top edge, and strictly above
       * the bottom edge.  (Some edges may be degenerate.)
       *
       * The following discussion assumes left-to-right scanning
       * (that is, the major edge is on the left); the right-to-left
       * case is a straightforward variation.
       *
       * We start by finding the half-integral y coordinate that is
       * at or below the top of the triangle.  This gives us the
       * first scan line that could possibly contain pixels that are
       * inside the triangle.
       *
       * Next we creep down the major edge until we reach that y,
       * and compute the corresponding x coordinate on the edge.
       * Then we find the half-integral x that lies on or just
       * inside the edge.  This is the first pixel that might lie in
       * the interior of the triangle.  (We won't know for sure
       * until we check the other edges.)
       *
       * As we rasterize the triangle, we'll step down the major
       * edge.  For each step in y, we'll move an integer number
       * of steps in x.  There are two possible x step sizes, which
       * we'll call the ``inner'' step (guaranteed to land on the
       * edge or inside it) and the ``outer'' step (guaranteed to
       * land on the edge or outside it).  The inner and outer steps
       * differ by one.  During rasterization we maintain an error
       * term that indicates our distance from the true edge, and
       * select either the inner step or the outer step, whichever
       * gets us to the first pixel that falls inside the triangle.
       *
       * All parameters (z, red, etc.) as well as the buffer
       * addresses for color and z have inner and outer step values,
       * so that we can increment them appropriately.  This method
       * eliminates the need to adjust parameters by creeping a
       * sub-pixel amount into the triangle at each scanline.
       */

      {
         int subTriangle;
         GLfixed fx;
         GLfixed fxLeftEdge = 0, fxRightEdge = 0;
         GLfixed fdxLeftEdge = 0, fdxRightEdge = 0;
         GLfixed fdxOuter;
         int idxOuter;
         float dxOuter;
         GLfixed fError = 0, fdError = 0;
         float adjx, adjy;
         GLfixed fy;
         GLushort *zRow = 0;
         int dZRowOuter = 0, dZRowInner;  /* offset in bytes */
         GLfixed fz = 0, fdzOuter = 0, fdzInner;

         for (subTriangle=0; subTriangle<=1; subTriangle++) {
            EdgeT *eLeft, *eRight;
            int setupLeft, setupRight;
            int lines;

            if (subTriangle==0) {
               /* bottom half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eBot;
                  lines = eRight->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
               else {
                  eLeft = &eBot;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 1;
               }
            }
            else {
               /* top half */
               if (scan_from_left_to_right) {
                  eLeft = &eMaj;
                  eRight = &eTop;
                  lines = eRight->lines;
                  setupLeft = 0;
                  setupRight = 1;
               }
               else {
                  eLeft = &eTop;
                  eRight = &eMaj;
                  lines = eLeft->lines;
                  setupLeft = 1;
                  setupRight = 0;
               }
               if (lines == 0)
                  return;
            }

            if (setupLeft && eLeft->lines > 0) {
               const SWvertex *vLower;
               GLfixed fsx = eLeft->fsx;
               fx = (((fsx) + 0x00000800 - 1) & (~0x000007FF));
               fError = fx - fsx - 0x00000800;
               fxLeftEdge = fsx - 1;
               fdxLeftEdge = eLeft->fdxdy;
               fdxOuter = ((fdxLeftEdge - 1) & (~0x000007FF));
               fdError = fdxOuter - fdxLeftEdge + 0x00000800;
               idxOuter = ((fdxOuter) >> 11);
               dxOuter = (float) idxOuter;
               (void) dxOuter;

               fy = eLeft->fsy;
               span.y = ((fy) >> 11);

               adjx = (float)(fx - eLeft->fx0);  /* SCALED! */
               adjy = eLeft->adjy;              /* SCALED! */
               vLower = eLeft->v0;

               /*
                * Now we need the set of parameter (z, color, etc.) values at
                * the point (fx, fy).  This gives us properly-sampled parameter
                * values that we can step from pixel to pixel.  Furthermore,
                * although we might have intermediate results that overflow
                * the normal parameter range when we step temporarily outside
                * the triangle, we shouldn't overflow or underflow for any
                * pixel that's actually inside the triangle.
                */

               {
                  GLfloat z0 = vLower->win[2];
                  if (depthBits <= 16) {
                     /* interpolate fixed-pt values */
                     GLfloat tmp = (z0 * 2048.0f +
                                    dzdx * adjx + dzdy * adjy) + 0x00000400;
                     if (tmp < 0xffffffff / 2)
                        fz = (GLfixed) tmp;
                     else
                        fz = 0xffffffff / 2;
                     fdzOuter = (((int) ((((dzdy + dxOuter * dzdx) * 2048.0f) >= 0.0F) ? (((dzdy + dxOuter * dzdx) * 2048.0f) + 0.5F) : (((dzdy + dxOuter * dzdx) * 2048.0f) - 0.5F))));
                  }
                  else {
                     /* interpolate depth values exactly */
                     fz = (GLint) (z0 + dzdx * ((adjx) * (1.0F / 2048.0f))
                                   + dzdy * ((adjy) * (1.0F / 2048.0f)));
                     fdzOuter = (GLint) (dzdy + dxOuter * dzdx);
                  }
                  zRow = (GLushort *)
                    _mesa_zbuffer_address(ctx, ((fxLeftEdge) >> 11), span.y);
                  dZRowOuter = (ctx->DrawBuffer->Width + idxOuter) * sizeof(GLushort);
               }

            } /*if setupLeft*/


            if (setupRight && eRight->lines>0) {
               fxRightEdge = eRight->fsx - 1;
               fdxRightEdge = eRight->fdxdy;
            }

            if (lines==0) {
               continue;
            }


            /* Rasterize setup */
            dZRowInner = dZRowOuter + sizeof(GLushort);
            fdzInner = fdzOuter + span.zStep;

            while (lines > 0) {
               /* initialize the span interpolants to the leftmost value */
               /* ff = fixed-pt fragment */
               const GLint right = ((fxRightEdge) >> 11);

               span.x = ((fxLeftEdge) >> 11);

               if (right <= span.x)
                  span.end = 0;
               else
                  span.end = right - span.x;

               span.z = fz;




               /* This is where we actually generate fragments */
               if (span.end > 0) {
                  GLuint i; for (i = 0; i < span.end; i++) { GLdepth z = ((span.z) >> fixedToDepthShift); if (z < zRow[i]) { ctx->OcclusionResult = 0x1; return; } span.z += span.zStep; };
               }

               /*
                * Advance to the next scan line.  Compute the
                * new edge coordinates, and adjust the
                * pixel-center x coordinate so that it stays
                * on or inside the major edge.
                */
               (span.y)++;
               lines--;

               fxLeftEdge += fdxLeftEdge;
               fxRightEdge += fdxRightEdge;


               fError += fdError;
               if (fError >= 0) {
                  fError -= 0x00000800;
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowOuter);
                  fz += fdzOuter;
               }
               else {
                  zRow = (GLushort *) ((GLubyte *) zRow + dZRowInner);
                  fz += fdzInner;
               }
            } /*while lines>0*/

         } /* for subTriangle */

      }
   }
}









static void
nodraw_triangle( GLcontext *ctx,
                 const SWvertex *v0,
                 const SWvertex *v1,
                 const SWvertex *v2 )
{
   (void) (ctx && v0 && v1 && v2);
}


/*
 * This is used when separate specular color is enabled, but not
 * texturing.  We add the specular color to the primary color,
 * draw the triangle, then restore the original primary color.
 * Inefficient, but seldom needed.
 */
void _swrast_add_spec_terms_triangle( GLcontext *ctx,
                                     const SWvertex *v0,
                                     const SWvertex *v1,
                                     const SWvertex *v2 )
{
   SWvertex *ncv0 = (SWvertex *)v0; /* drop const qualifier */
   SWvertex *ncv1 = (SWvertex *)v1;
   SWvertex *ncv2 = (SWvertex *)v2;
   GLint rSum, gSum, bSum;
   GLchan c[3][4];
   /* save original colors */
   { (c[0])[0] = (ncv0->color)[0]; (c[0])[1] = (ncv0->color)[1]; (c[0])[2] = (ncv0->color)[2]; (c[0])[3] = (ncv0->color)[3]; };
   { (c[1])[0] = (ncv1->color)[0]; (c[1])[1] = (ncv1->color)[1]; (c[1])[2] = (ncv1->color)[2]; (c[1])[3] = (ncv1->color)[3]; };
   { (c[2])[0] = (ncv2->color)[0]; (c[2])[1] = (ncv2->color)[1]; (c[2])[2] = (ncv2->color)[2]; (c[2])[3] = (ncv2->color)[3]; };
   /* sum v0 */
   rSum = ncv0->color[0] + ncv0->specular[0];
   gSum = ncv0->color[1] + ncv0->specular[1];
   bSum = ncv0->color[2] + ncv0->specular[2];
   ncv0->color[0] = ( (rSum)<(255) ? (rSum) : (255) );
   ncv0->color[1] = ( (gSum)<(255) ? (gSum) : (255) );
   ncv0->color[2] = ( (bSum)<(255) ? (bSum) : (255) );
   /* sum v1 */
   rSum = ncv1->color[0] + ncv1->specular[0];
   gSum = ncv1->color[1] + ncv1->specular[1];
   bSum = ncv1->color[2] + ncv1->specular[2];
   ncv1->color[0] = ( (rSum)<(255) ? (rSum) : (255) );
   ncv1->color[1] = ( (gSum)<(255) ? (gSum) : (255) );
   ncv1->color[2] = ( (bSum)<(255) ? (bSum) : (255) );
   /* sum v2 */
   rSum = ncv2->color[0] + ncv2->specular[0];
   gSum = ncv2->color[1] + ncv2->specular[1];
   bSum = ncv2->color[2] + ncv2->specular[2];
   ncv2->color[0] = ( (rSum)<(255) ? (rSum) : (255) );
   ncv2->color[1] = ( (gSum)<(255) ? (gSum) : (255) );
   ncv2->color[2] = ( (bSum)<(255) ? (bSum) : (255) );
   /* draw */
   ((SWcontext *)ctx->swrast_context)->SpecTriangle( ctx, ncv0, ncv1, ncv2 );
   /* restore original colors */
   { (ncv0->color)[0] = (c[0])[0]; (ncv0->color)[1] = (c[0])[1]; (ncv0->color)[2] = (c[0])[2]; (ncv0->color)[3] = (c[0])[3]; };
   { (ncv1->color)[0] = (c[1])[0]; (ncv1->color)[1] = (c[1])[1]; (ncv1->color)[2] = (c[1])[2]; (ncv1->color)[3] = (c[1])[3]; };
   { (ncv2->color)[0] = (c[2])[0]; (ncv2->color)[1] = (c[2])[1]; (ncv2->color)[2] = (c[2])[2]; (ncv2->color)[3] = (c[2])[3]; };
}









/*
 * Determine which triangle rendering function to use given the current
 * rendering context.
 *
 * Please update the summary flag _SWRAST_NEW_TRIANGLE if you add or
 * remove tests to this code.
 */
void
_swrast_choose_triangle( GLcontext *ctx )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);
   const GLboolean rgbmode = ctx->Visual.rgbMode;

   if (ctx->Polygon.CullFlag &&
       ctx->Polygon.CullFaceMode == 0x0408) {
      swrast->Triangle = nodraw_triangle;;
      return;
   }

   if (ctx->RenderMode==0x1C00) {

      if (ctx->Polygon.SmoothFlag) {
         _mesa_set_aa_triangle_function(ctx);
         ;
         return;
      }

      if (ctx->Depth.OcclusionTest &&
          ctx->Depth.Test &&
          ctx->Depth.Mask == 0x0 &&
          ctx->Depth.Func == 0x0201 &&
          !ctx->Stencil.Enabled) {
         if ((rgbmode &&
              ctx->Color.ColorMask[0] == 0 &&
              ctx->Color.ColorMask[1] == 0 &&
              ctx->Color.ColorMask[2] == 0 &&
              ctx->Color.ColorMask[3] == 0)
             ||
             (!rgbmode && ctx->Color.IndexMask == 0)) {
            swrast->Triangle = occlusion_zless_triangle;;
            return;
         }
      }

      if (ctx->Texture._EnabledUnits) {
         /* Ugh, we do a _lot_ of tests to pick the best textured tri func */
        const struct gl_texture_object *texObj2D;
         const struct gl_texture_image *texImg;
         GLenum minFilter, magFilter, envMode;
         GLint format;
         texObj2D = ctx->Texture.Unit[0].Current2D;
         texImg = texObj2D ? texObj2D->Image[texObj2D->BaseLevel] : 0;
         format = texImg ? texImg->TexFormat->MesaFormat : -1;
         minFilter = texObj2D ? texObj2D->MinFilter : (GLenum) 0;
         magFilter = texObj2D ? texObj2D->MagFilter : (GLenum) 0;
         envMode = ctx->Texture.Unit[0].EnvMode;

         /* First see if we can used an optimized 2-D texture function */
         if (ctx->Texture._EnabledUnits == 1
             && ctx->Texture.Unit[0]._ReallyEnabled == 0x02
             && texObj2D->WrapS==0x2901
            && texObj2D->WrapT==0x2901
             && texImg->Border==0
             && texImg->Width == texImg->RowStride
             && (format == MESA_FORMAT_RGB || format == MESA_FORMAT_RGBA)
            && minFilter == magFilter
            && ctx->Light.Model.ColorControl == 0x81F9
            && ctx->Texture.Unit[0].EnvMode != 0x8570) {
           if (ctx->Hint.PerspectiveCorrection==0x1101) {
              if (minFilter == 0x2600
                  && format == MESA_FORMAT_RGB
                  && (envMode == 0x1E01 || envMode == 0x2101)
                  && ((swrast->_RasterMask == (0x004 | 0x1000)
                       && ctx->Depth.Func == 0x0201
                       && ctx->Depth.Mask == 0x1)
                      || swrast->_RasterMask == 0x1000)
                  && ctx->Polygon.StippleFlag == 0x0) {
                 if (swrast->_RasterMask == (0x004 | 0x1000)) {
                    swrast->Triangle = simple_z_textured_triangle;
                 }
                 else {
                    swrast->Triangle = simple_textured_triangle;
                 }
              }
              else {
                  swrast->Triangle = affine_textured_triangle;
              }
           }
           else {
               swrast->Triangle = persp_textured_triangle;
           }
        }
         else {
            /* general case textured triangles */
            if (ctx->Texture._EnabledUnits > 1) {
               swrast->Triangle = multitextured_triangle;
            }
            else {
               swrast->Triangle = general_textured_triangle;
            }
         }
      }
      else {
         ;
        if (ctx->Light.ShadeModel==0x1D01) {
           /* smooth shaded, no texturing, stippled or some raster ops */
            if (rgbmode) {
              swrast->Triangle = smooth_rgba_triangle;
            }
            else {
               swrast->Triangle = smooth_ci_triangle;
            }
        }
        else {
           /* flat shaded, no texturing, stippled or some raster ops */
            if (rgbmode) {
              swrast->Triangle = flat_rgba_triangle;
            }
            else {
               swrast->Triangle = flat_ci_triangle;;
            }
        }
      }
   }
   else if (ctx->RenderMode==0x1C01) {
      swrast->Triangle = _mesa_feedback_triangle;;
   }
   else {
      /* GL_SELECT mode */
      swrast->Triangle = _mesa_select_triangle;;
   }
}
