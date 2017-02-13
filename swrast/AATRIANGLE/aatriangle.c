/* aatriangle.c  */
/* originally made from aatriangle.c and aatritemp.h via 'icc -Pc+ -Pe+ aatriangle.c' + edit - EK */
/* with great optimization made for rgba_aa_tri */
/* $Id: aatriangle.c,v 1.5 2000/04/05 14:41:08 brianp Exp $ */
/* $Id: aatritemp.h,v 1.9 2000/04/01 05:42:06 brianp Exp $ */
/*
 * Mesa 3-D graphics library
 * Version:  3.3
 *
 * Copyright (C) 1999-2000  Brian Paul   All Rights Reserved.
 * partial (C) 2000 Evgeny Kotsuba
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
 * Antialiased Triangle rasterizers
 */

#ifdef PC_HEADER
#include "all.h"
#else
#include "glheader.h"
#include "aatriangle.h"
#include "span.h"
#include "types.h"
#include "vb.h"
#endif


/* EK */
#include "malloc.h"
#include "debug_mem.h"
#define FIST_MAGIC ((((65536.0 * 65536.0 * 16)+(65536.0 * 0.5))* 65536.0))
#define INLINE _Inline
/*
 * Compute coefficients of a plane using the X,Y coords of the v0, v1, v2
 * vertices and the given Z values.
 */
/* static */
static void  INLINE
compute_plane(const GLfloat v0[], const GLfloat v1[], const GLfloat v2[],
              GLfloat z0, GLfloat z1, GLfloat z2, GLfloat plane[4])
{
   const GLfloat px = v1[0] - v0[0];
   const GLfloat py = v1[1] - v0[1];
   const GLfloat pz = z1 - z0;

   const GLfloat qx = v2[0] - v0[0];
   const GLfloat qy = v2[1] - v0[1];
   const GLfloat qz = z2 - z0;

   const GLfloat a = py * qz - pz * qy;
   const GLfloat b = pz * qx - px * qz;
   const GLfloat c = px * qy - py * qx;
   const GLfloat d = -(a * v0[0] + b * v0[1] + c * z0);

   plane[0] = a;
   plane[1] = b;
   plane[2] = c;
   plane[3] = d;
}


/*
 * Compute coefficients of a plane with a constant Z value.
 */
/* static */
static void  INLINE
constant_plane(GLfloat value, GLfloat plane[4])
{
   plane[0] = 0.0;
   plane[1] = 0.0;
   plane[2] = -1.0;
   plane[3] = value;
}




/*
 * Solve plane equation for Z at (X,Y).
 */
/* static */
static  INLINE GLfloat
solve_plane(GLfloat x, GLfloat y, const GLfloat plane[4])
{
   GLfloat z = (plane[3] + plane[0] * x + plane[1] * y) / -plane[2];
   return z;
}




/*
 * Return 1 / solve_plane().
 */
/* static */
static  INLINE GLfloat
solve_plane_recip(GLfloat x, GLfloat y, const GLfloat plane[4])
{
   GLfloat z = -plane[2] / (plane[3] + plane[0] * x + plane[1] * y);
   return z;
}


/*
 * Solve plane and return clamped GLubyte value.
 */
/* static */
static  INLINE GLubyte
solve_plane_0_255(GLfloat x, GLfloat y, const GLfloat plane[4])
{
   GLfloat z = (plane[3] + plane[0] * x + plane[1] * y) / -plane[2] + 0.5F;
   if (z < 0.0F)
      return 0;
   else if (z > 255.0F)
      return 255;
   return (GLubyte) (GLint) z;
}


/*
 * Compute how much (area) of the given pixel is inside the triangle.
 * Vertices MUST be specified in counter-clockwise order.
 * Return:  coverage in [0, 1].
 */
/* static */
static  GLfloat
compute_coveragef(const GLfloat v0[3], const GLfloat v1[3],
                  const GLfloat v2[3], GLint winx, GLint winy)
{
   static const GLfloat samples[16][2] = {
      /* start with the four corners */
      { 0.00, 0.00 },
      { 0.75, 0.00 },
      { 0.00, 0.75 },
      { 0.75, 0.75 },
      /* continue with interior samples */
      { 0.25, 0.00 },
      { 0.50, 0.00 },
      { 0.00, 0.25 },
      { 0.25, 0.25 },
      { 0.50, 0.25 },
      { 0.75, 0.25 },
      { 0.00, 0.50 },
      { 0.25, 0.50 },
      { 0.50, 0.50 },
      { 0.75, 0.50 },
      { 0.25, 0.75 },
      { 0.50, 0.75 }
   };
   const GLfloat x = (GLfloat) winx;
   const GLfloat y = (GLfloat) winy;
   const GLfloat dx0 = v1[0] - v0[0];
   const GLfloat dy0 = v1[1] - v0[1];
   const GLfloat dx1 = v2[0] - v1[0];
   const GLfloat dy1 = v2[1] - v1[1];
   const GLfloat dx2 = v0[0] - v2[0];
   const GLfloat dy2 = v0[1] - v2[1];
   GLfloat cross0,cross1,cross2;
   GLint stop = 4, i;
   GLfloat insideCount = 16.0F;


   for (i = 0; i < stop; i++) {
      const GLfloat sx = x + samples[i][0];
      const GLfloat sy = y + samples[i][1];
//      const GLfloat fx0 = sx - v0[0];
//      const GLfloat fy0 = sy - v0[1];
//      const GLfloat fx1 = sx - v1[0];
//      const GLfloat fy1 = sy - v1[1];
//      const GLfloat fx2 = sx - v2[0];
//      const GLfloat fy2 = sy - v2[1];

//      fx0 = sx - v0[0];
//      fy0 = sy - v0[1];
      /* cross product determines if sample is inside or outside each edge */
      cross0 = (dx0 * (sy - v0[1]) - dy0 * (sx - v0[0]));
      /* Check if the sample is exactly on an edge.  If so, let cross be a
       * positive or negative value depending on the direction of the edge.
       */
      if (cross0 == 0.0F)
         cross0 = dx0 + dy0;
      if (cross0 < 0.0F) {
         /* point is outside triangle */
         insideCount--;
         stop = 16;
         continue;
      }
//      fx1 = sx - v1[0];
//      fy1 = sy - v1[1];

//      cross1 = (dx1 * fy1 - dy1 * fx1);
      cross1 = (dx1 * (sy - v1[1]) - dy1 * (sx - v1[0]));

      if (cross1 == 0.0F)
         cross1 = dx1 + dy1;
      if (cross1 < 0.0F) {
         /* point is outside triangle */
         insideCount--;
         stop = 16;
         continue;
      }
//      fx2 = sx - v2[0];
//      fy2 = sy - v2[1];
//    cross2 = (dx2 * fy2 - dy2 * fx2);
      cross2 = (dx2 * (sy - v2[1]) - dy2 * (sx - v2[0]));
      if (cross2 == 0.0F)
         cross2 = dx2 + dy2;
      if (cross2 < 0.0F) {
         /* point is outside triangle */
         insideCount--;
         stop = 16;
      }
   }

   if (stop == 4)
      return 1.0F;
   else
      return insideCount * (1.0F / 16.0F);

}


/*
 * Compute how much (area) of the given pixel is inside the triangle.
 * Vertices MUST be specified in counter-clockwise order.
 * Return:  coverage in [0, 15].
 */

static  GLint
compute_coveragei(const GLfloat v0[3], const GLfloat v1[3],
                  const GLfloat v2[3], GLint winx, GLint winy)
{
   /* NOTE: 15 samples instead of 16.
    * A better sample distribution could be used.
    */
   static const GLfloat samples[15][2] = {
      /* start with the four corners */
      { 0.00, 0.00 },
      { 0.75, 0.00 },
      { 0.00, 0.75 },
      { 0.75, 0.75 },
      /* continue with interior samples */
      { 0.25, 0.00 },
      { 0.50, 0.00 },
      { 0.00, 0.25 },
      { 0.25, 0.25 },
      { 0.50, 0.25 },
      { 0.75, 0.25 },
      { 0.00, 0.50 },
      { 0.25, 0.50 },
      /*{ 0.50, 0.50 },*/
      { 0.75, 0.50 },
      { 0.25, 0.75 },
      { 0.50, 0.75 }
   };
   const GLfloat x = (GLfloat) winx;
   const GLfloat y = (GLfloat) winy;
   const GLfloat dx0 = v1[0] - v0[0];
   const GLfloat dy0 = v1[1] - v0[1];
   const GLfloat dx1 = v2[0] - v1[0];
   const GLfloat dy1 = v2[1] - v1[1];
   const GLfloat dx2 = v0[0] - v2[0];
   const GLfloat dy2 = v0[1] - v2[1];
   GLint stop = 4, i;
   GLint insideCount = 15;


   for (i = 0; i < stop; i++) {
      const GLfloat sx = x + samples[i][0];
      const GLfloat sy = y + samples[i][1];
      const GLfloat fx0 = sx - v0[0];
      const GLfloat fy0 = sy - v0[1];
      const GLfloat fx1 = sx - v1[0];
      const GLfloat fy1 = sy - v1[1];
      const GLfloat fx2 = sx - v2[0];
      const GLfloat fy2 = sy - v2[1];
      /* cross product determines if sample is inside or outside each edge */
      GLfloat cross0 = (dx0 * fy0 - dy0 * fx0);
      GLfloat cross1 = (dx1 * fy1 - dy1 * fx1);
      GLfloat cross2 = (dx2 * fy2 - dy2 * fx2);
      /* Check if the sample is exactly on an edge.  If so, let cross be a
       * positive or negative value depending on the direction of the edge.
       */
      if (cross0 == 0.0F)
         cross0 = dx0 + dy0;
      if (cross1 == 0.0F)
         cross1 = dx1 + dy1;
      if (cross2 == 0.0F)
         cross2 = dx2 + dy2;
      if (cross0 < 0.0F || cross1 < 0.0F || cross2 < 0.0F) {
         /* point is outside triangle */
         insideCount--;
         stop = 15;
      }
   }
   if (stop == 4)
      return 15;
   else
      return insideCount;
}

/* $Id: aatritemp.h,v 1.6 2000/03/14 15:05:47 brianp Exp $ */
/*
 * Antialiased Triangle Rasterizer Template
 *
 * This file is #include'd to generate custom AA triangle rasterizers.
 * NOTE: this code hasn't been optimized yet.  That'll come after it
 * works correctly.
 *
 * The following macros may be defined to indicate what auxillary information
 * must be copmuted across the triangle:
 *    DO_Z      - if defined, compute Z values
 *    DO_RGBA   - if defined, compute RGBA values
 *    DO_INDEX  - if defined, compute color index values
 *    DO_SPEC   - if defined, compute specular RGB values
 *    DO_STUV0  - if defined, compute unit 0 STRQ texcoords
 *    DO_STUV1  - if defined, compute unit 1 STRQ texcoords
 */

/*void triangle( GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv )*/

//#define DO_Z
//#define DO_RGBA

static void
rgba_aa_tri(GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
   const struct vertex_buffer *VB = ctx->VB;
   const GLfloat *p0 = VB->Win.data[v0];
   const GLfloat *p1 = VB->Win.data[v1];
   const GLfloat *p2 = VB->Win.data[v2];
   GLint vMin, vMid, vMax;
   GLint iyMin, iyMax,ixMin,ixMax;
   GLfloat yMin, yMax,xMax,xMin;
   GLboolean ltor;
   GLfloat majDx, majDy;
   GLfloat zPlane[4],zPlaneDelta;                           /* Z (depth) */
   GLdepth z[MAX_WIDTH];
   GLfloat rPlane[4], gPlane[4], bPlane[4], aPlane[4];      /* color */
   GLfloat rPlaneDelta,gPlaneDelta,bPlaneDelta,aPlaneDelta;
   GLubyte rgba[MAX_WIDTH][4];
   GLfloat bf = ctx->backface_sign;

   float ival;  /* EK */
   double dtemp;
   float  rPlane_c0, gPlane_c0,bPlane_c0,aPlane_c0, zPlane_c0,tmpz;
   GLuint itmpz;

   /* determine bottom to top order of vertices */

   GLfloat y0 = VB->Win.data[v0][1];
   GLfloat y1 = VB->Win.data[v1][1];
   GLfloat y2 = VB->Win.data[v2][1];

      if (y0 <= y1) {
        if (y1 <= y2) {
           vMin = v0;   vMid = v1;   vMax = v2;   /* y0<=y1<=y2 */
        }
        else if (y2 <= y0) {
           vMin = v2;   vMid = v0;   vMax = v1;   /* y2<=y0<=y1 */
        }
        else {
           vMin = v0;   vMid = v2;   vMax = v1;  bf = -bf; /* y0<=y2<=y1 */
        }
      }
      else {
        if (y0 <= y2) {
           vMin = v1;   vMid = v0;   vMax = v2;  bf = -bf; /* y1<=y0<=y2 */
        }
        else if (y2 <= y1) {
           vMin = v2;   vMid = v1;   vMax = v0;  bf = -bf; /* y2<=y1<=y0 */
        }
        else {
           vMin = v1;   vMid = v2;   vMax = v0;   /* y1<=y2<=y0 */
        }
      }

   majDx = VB->Win.data[vMax][0] - VB->Win.data[vMin][0];
   majDy = VB->Win.data[vMax][1] - VB->Win.data[vMin][1];

   {
      const GLfloat botDx = VB->Win.data[vMid][0] - VB->Win.data[vMin][0];
      const GLfloat botDy = VB->Win.data[vMid][1] - VB->Win.data[vMin][1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area * area < .0025)
        return;
   }

#ifndef DO_OCCLUSION_TEST
   ctx->OcclusionResult = GL_TRUE;
#endif

   /* plane setup */
   compute_plane(p0, p1, p2, p0[2], p1[2], p2[2], zPlane);
   if (ctx->Light.ShadeModel == GL_SMOOTH) {
      GLubyte (*rgba)[4] = VB->ColorPtr->data;
      compute_plane(p0, p1, p2, rgba[v0][0], rgba[v1][0], rgba[v2][0], rPlane);
      compute_plane(p0, p1, p2, rgba[v0][1], rgba[v1][1], rgba[v2][1], gPlane);
      compute_plane(p0, p1, p2, rgba[v0][2], rgba[v1][2], rgba[v2][2], bPlane);
      compute_plane(p0, p1, p2, rgba[v0][3], rgba[v1][3], rgba[v2][3], aPlane);
   }
   else {
      constant_plane(VB->ColorPtr->data[pv][0], rPlane);
      constant_plane(VB->ColorPtr->data[pv][1], gPlane);
      constant_plane(VB->ColorPtr->data[pv][2], bPlane);
      constant_plane(VB->ColorPtr->data[pv][3], aPlane);
   }

   yMin = VB->Win.data[vMin][1];
   yMax = VB->Win.data[vMax][1];
   iyMin = (int) yMin;
   iyMax = (int) yMax + 1;
/* EK */
   xMax = xMin = VB->Win.data[vMax][0];
   if(VB->Win.data[vMin][0] > xMax) xMax = VB->Win.data[vMin][0];
   if(VB->Win.data[vMin][0] < xMin) xMin = VB->Win.data[vMin][0];
   if(VB->Win.data[vMid][0] > xMax) xMax = VB->Win.data[vMid][0];
   if(VB->Win.data[vMid][0] < xMin) xMin = VB->Win.data[vMid][0];
   ixMin = (int) xMin;
   ixMax = (int) xMax + 1;
//   if(ixMin < 0) ixMin = 0;
   if(ixMax >= ctx->DrawBuffer->Width) ixMax = ctx->DrawBuffer->Width - 1;
   if(iyMax >= ctx->DrawBuffer->Height) iyMax = ctx->DrawBuffer->Height - 1;

   if (ltor) {
      /* scan left to right */
      const float *pMin = VB->Win.data[vMin];
      const float *pMid = VB->Win.data[vMid];
      const float *pMax = VB->Win.data[vMax];
      const float dxdy = majDx / majDy;
      const float xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      float x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      int iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count, n;
         GLfloat coverage=0.f;
         /* skip over fragments with zero coverage */
         while (startX < ixMax  /* MAX_WIDTH EK */ ) {
            coverage = compute_coveragef(pMin, pMid, pMax, startX, iy);
            if (coverage > 0.0F)
               break;
            startX++;
         }

         /* enter interior of triangle */
         ix = startX;
         if (coverage > 0.0F)
         {
         count = 0;
//   rPlane_c0 = rPlane[3]+rPlane[1]*iy;
   rPlane_c0 = ( rPlane[3]+rPlane[1]*iy + rPlane[0]*ix)/ -rPlane[2];
   rPlaneDelta = rPlane[0]/ -rPlane[2];
   gPlane_c0 = ( gPlane[3]+gPlane[1]*iy + gPlane[0]*ix)/ -gPlane[2];
   gPlaneDelta = gPlane[0]/ -gPlane[2];
   bPlane_c0 = ( bPlane[3]+bPlane[1]*iy + bPlane[0]*ix)/ -bPlane[2];
   bPlaneDelta = bPlane[0]/ -bPlane[2];
   aPlane_c0 = ( aPlane[3]+aPlane[1]*iy + aPlane[0]*ix)/ -aPlane[2];
   aPlaneDelta = aPlane[0]/ -aPlane[2];

//   zPlane_c0 = zPlane[3]+zPlane[1]*iy;
   zPlane_c0 = ( zPlane[3] + zPlane[1]*iy + zPlane[0]*ix) / -zPlane[2];
   zPlaneDelta = zPlane[0] / -zPlane[2];
             for(;ix <ixMax ;)
             {
////         while (coverage > 0.0F) {
//            z[count] = (GLdepth) solve_plane(ix, iy, zPlane);
//GLdepth = int or short int
              dtemp = FIST_MAGIC + zPlane_c0;
              z[count] = (GLdepth) ((*(int *)&dtemp) - 0x80000000);
//          rgba[count][RCOMP] = solve_plane_0_255(ix, iy, rPlane);
              if (rPlane_c0 < 0.0F)        itmpz = 0;
              else if (rPlane_c0 > 255.0F)      itmpz = 255;
              else
              {  //itmpz = tmpz;
                  dtemp = FIST_MAGIC + rPlane_c0;
                  itmpz = (GLubyte) ((*(int *)&dtemp) - 0x80000000);
              }
              rgba[count][RCOMP] = (GLubyte)itmpz;
//          rgba[count][GCOMP] = solve_plane_0_255(ix, iy, gPlane);
              if (gPlane_c0 < 0.0F)             itmpz = 0;
              else if (gPlane_c0 > 255.0F)      itmpz = 255;
              else
              {  //itmpz = tmpz;
                 dtemp = FIST_MAGIC + gPlane_c0;
                 itmpz = (GLubyte) ((*(int *)&dtemp) - 0x80000000);
              }
              rgba[count][GCOMP] = (GLubyte)itmpz;

//            rgba[count][BCOMP] = solve_plane_0_255(ix, iy, bPlane);
              if (bPlane_c0 < 0.0F)             itmpz = 0;
              else if (bPlane_c0 > 255.0F)      itmpz = 255;
              else
              {  //itmpz = tmpz;
                 dtemp = FIST_MAGIC + bPlane_c0;
                 itmpz = (GLubyte) ((*(int *)&dtemp) - 0x80000000);
              }
              rgba[count][BCOMP] = (GLubyte)itmpz;

//          rgba[count][ACOMP] = solve_plane_0_255(ix, iy, aPlane) * coverage;
//          tmpz = (aPlane_c0 + aPlane[0]*ix)/ -aPlane[2] * coverage; /* + 0.5F; */
              tmpz = aPlane_c0 * coverage;
              if (tmpz < 0.0F)             itmpz = 0;
              else if (tmpz > 255.0F)      itmpz = 255;
              else
              {  //itmpz = tmpz;
                 dtemp = FIST_MAGIC + tmpz;
                 itmpz = (GLubyte) ((*(int *)&dtemp) - 0x80000000);
              }
              rgba[count][ACOMP] = (GLubyte)itmpz;

              ix++;
              coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
              if(!(coverage > 0.0F))
                      break;
              count++;
              zPlane_c0 += zPlaneDelta;
              rPlane_c0 += rPlaneDelta;
              gPlane_c0 += gPlaneDelta;
              bPlane_c0 += bPlaneDelta;
              aPlane_c0 += aPlaneDelta;
            }
         } // endif (coverage > 0.0F)

         n = (GLuint) ix - (GLuint) startX;
//  if(n == 2 && startX == 199 && iy == 199)
//         printf("n=%i,startX=%i,iy=%i\n",n,startX,iy);
         gl_write_rgba_span(ctx, n, startX, iy, z, rgba, GL_POLYGON);
//CHECK_MEM;
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = VB->Win.data[vMin];
      const GLfloat *pMid = VB->Win.data[vMid];
      const GLfloat *pMax = VB->Win.data[vMax];
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage =0.f;
         /* skip fragments with zero coverage */
         while (startX >= ixMin /* 0 EK */) {
            coverage = compute_coveragef(pMin, pMax, pMid, startX, iy);
            if (coverage > 0.0F)
               break;
            startX--;
         }

         /* enter interior of triangle */
         ix = startX;
         if (coverage > 0.0F)
         {
           rPlane_c0 = ( rPlane[3]+rPlane[1]*iy + rPlane[0]*ix)/ -rPlane[2];
           rPlaneDelta = rPlane[0]/ rPlane[2];
           gPlane_c0 = ( gPlane[3]+gPlane[1]*iy + gPlane[0]*ix)/ -gPlane[2];
           gPlaneDelta = gPlane[0]/ gPlane[2];
           bPlane_c0 = ( bPlane[3]+bPlane[1]*iy + bPlane[0]*ix)/ -bPlane[2];
           bPlaneDelta = bPlane[0]/ bPlane[2];
           aPlane_c0 = ( aPlane[3]+aPlane[1]*iy + aPlane[0]*ix)/ -aPlane[2];
           aPlaneDelta = aPlane[0]/ aPlane[2];
           zPlane_c0 = ( zPlane[3] + zPlane[1]*iy+zPlane[0]*ix) / -zPlane[2];
           zPlaneDelta = zPlane[0] / zPlane[2];

           for(;ix>0;)
//while (coverage > 0.0F)
           {
//            z[ix] = (GLdepth) solve_plane(ix, iy, zPlane);
             dtemp = FIST_MAGIC + zPlane_c0;
             z[ix] = (GLdepth) ((*(int *)&dtemp) - 0x80000000);

//          rgba[ix][RCOMP] = solve_plane_0_255(ix, iy, rPlane);
             if (rPlane_c0 < 0.0F)             itmpz = 0;
             else if (rPlane_c0 > 255.0F)      itmpz = 255;
             else
             {  //itmpz = tmpz;
                dtemp = FIST_MAGIC + rPlane_c0;
                itmpz = (GLubyte) ((*(int *)&dtemp) - 0x80000000);
             }

             rgba[ix][RCOMP] = (GLubyte)itmpz;

//          rgba[ix][GCOMP] = solve_plane_0_255(ix, iy, gPlane);
             if (gPlane_c0 < 0.0F)             itmpz = 0;
             else if (gPlane_c0 > 255.0F)      itmpz = 255;
             else
             {  //itmpz = tmpz;
                dtemp = FIST_MAGIC + gPlane_c0;
                itmpz = (GLubyte) ((*(int *)&dtemp) - 0x80000000);
             }
             rgba[ix][GCOMP] = (GLubyte)itmpz;

//            rgba[ix][BCOMP] = solve_plane_0_255(ix, iy, bPlane);
             if (bPlane_c0 < 0.0F)             itmpz = 0;
             else if (bPlane_c0 > 255.0F)      itmpz = 255;
             else
             {  //itmpz = tmpz;
                dtemp = FIST_MAGIC + bPlane_c0;
                itmpz = (GLubyte) ((*(int *)&dtemp) - 0x80000000);
             }
             rgba[ix][BCOMP] = (GLubyte)itmpz;

//          rgba[ix][ACOMP] = solve_plane_0_255(ix, iy, aPlane) * coverage;
             tmpz = aPlane_c0 * coverage;
             if (tmpz < 0.0F)             itmpz = 0;
             else if (tmpz > 255.0F)      itmpz = 255;
             else
             {  //itmpz = tmpz;
                dtemp = FIST_MAGIC + tmpz;
                itmpz = (GLubyte) ((*(int *)&dtemp) - 0x80000000);
             }
             rgba[ix][ACOMP] = (GLubyte)itmpz;

             ix--;
             coverage = compute_coveragef(pMin, pMax, pMid, ix, iy);
             if(!(coverage > 0.0F))
                      break;
             zPlane_c0 += zPlaneDelta;
             rPlane_c0 += rPlaneDelta;
             gPlane_c0 += gPlaneDelta;
             bPlane_c0 += bPlaneDelta;
             aPlane_c0 += aPlaneDelta;
           }
         } //endif (coverage > 0.0F)

         n = (GLuint) startX - (GLuint) ix;
         left = ix + 1;
//  printf("n=%i,startX=%i,iy=%i\n",n,startX,iy);
         gl_write_rgba_span(ctx, n, left, iy, z + left,
                            rgba + left, GL_POLYGON);
//CHECK_MEM;
      }
   }
}


//#define DO_Z
//#define DO_INDEX

static void
index_aa_tri(GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
   const struct vertex_buffer *VB = ctx->VB;
   const GLfloat *p0 = VB->Win.data[v0];
   const GLfloat *p1 = VB->Win.data[v1];
   const GLfloat *p2 = VB->Win.data[v2];
   GLint vMin, vMid, vMax;
   GLint iyMin, iyMax;
   GLfloat yMin, yMax;
   GLboolean ltor;
   GLfloat majDx, majDy;
   GLfloat zPlane[4];                                       /* Z (depth) */
   GLdepth z[MAX_WIDTH];
   GLfloat iPlane[4];                                       /* color index */
   GLuint index[MAX_WIDTH];
   GLfloat bf = ctx->backface_sign;

   /* determine bottom to top order of vertices */
   {
      GLfloat y0 = VB->Win.data[v0][1];
      GLfloat y1 = VB->Win.data[v1][1];
      GLfloat y2 = VB->Win.data[v2][1];
      if (y0 <= y1) {
        if (y1 <= y2) {
           vMin = v0;   vMid = v1;   vMax = v2;   /* y0<=y1<=y2 */
        }
        else if (y2 <= y0) {
           vMin = v2;   vMid = v0;   vMax = v1;   /* y2<=y0<=y1 */
        }
        else {
           vMin = v0;   vMid = v2;   vMax = v1;  bf = -bf; /* y0<=y2<=y1 */
        }
      }
      else {
        if (y0 <= y2) {
           vMin = v1;   vMid = v0;   vMax = v2;  bf = -bf; /* y1<=y0<=y2 */
        }
        else if (y2 <= y1) {
           vMin = v2;   vMid = v1;   vMax = v0;  bf = -bf; /* y2<=y1<=y0 */
        }
        else {
           vMin = v1;   vMid = v2;   vMax = v0;   /* y1<=y2<=y0 */
        }
      }
   }

   majDx = VB->Win.data[vMax][0] - VB->Win.data[vMin][0];
   majDy = VB->Win.data[vMax][1] - VB->Win.data[vMin][1];

   {
      const GLfloat botDx = VB->Win.data[vMid][0] - VB->Win.data[vMin][0];
      const GLfloat botDy = VB->Win.data[vMid][1] - VB->Win.data[vMin][1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area * area < .0025)
        return;
   }

#ifndef DO_OCCLUSION_TEST
   ctx->OcclusionResult = GL_TRUE;
#endif

   /* plane setup */
   compute_plane(p0, p1, p2, p0[2], p1[2], p2[2], zPlane);
   if (ctx->Light.ShadeModel == GL_SMOOTH) {
      compute_plane(p0, p1, p2, VB->IndexPtr->data[v0],
                    VB->IndexPtr->data[v1], VB->IndexPtr->data[v2], iPlane);
   }
   else {
      constant_plane(VB->IndexPtr->data[pv], iPlane);
   }

   yMin = VB->Win.data[vMin][1];
   yMax = VB->Win.data[vMax][1];
   iyMin = (int) yMin;
   iyMax = (int) yMax + 1;

   if (ltor) {
      /* scan left to right */
      const float *pMin = VB->Win.data[vMin];
      const float *pMid = VB->Win.data[vMid];
      const float *pMax = VB->Win.data[vMax];
      const float dxdy = majDx / majDy;
      const float xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      float x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      int iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count, n;
         GLfloat coverage=0.f;
         /* skip over fragments with zero coverage */
         while (startX < MAX_WIDTH) {
            coverage = compute_coveragef(pMin, pMid, pMax, startX, iy);
            if (coverage > 0.0F)
               break;
            startX++;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            z[count] = (GLdepth) solve_plane(ix, iy, zPlane);
            {
               GLint frac = compute_coveragei(pMin, pMid, pMax, ix, iy);
               GLint indx = (GLint) solve_plane(ix, iy, iPlane);
               index[count] = (indx & ~0xf) | frac;
            }
            ix++;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         n = (GLuint) ix - (GLuint) startX;
         gl_write_index_span(ctx, n, startX, iy, z, index, GL_POLYGON);
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = VB->Win.data[vMin];
      const GLfloat *pMid = VB->Win.data[vMid];
      const GLfloat *pMax = VB->Win.data[vMax];
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage=0.f;
         /* skip fragments with zero coverage */
         while (startX >= 0) {
            coverage = compute_coveragef(pMin, pMax, pMid, startX, iy);
            if (coverage > 0.0F)
               break;
            startX--;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            z[ix] = (GLdepth) solve_plane(ix, iy, zPlane);
            {
               GLint frac = compute_coveragei(pMin, pMax, pMid, ix, iy);
               GLint indx = (GLint) solve_plane(ix, iy, iPlane);
               index[ix] = (indx & ~0xf) | frac;
            }
            ix--;
            count++;
            coverage = compute_coveragef(pMin, pMax, pMid, ix, iy);
         }

         n = (GLuint) startX - (GLuint) ix;
         left = ix + 1;
         gl_write_index_span(ctx, n, left, iy, z + left,
                             index + left, GL_POLYGON);
      }
   }
}


/*
 * Compute mipmap level of detail.
 */
static INLINE GLfloat
compute_lambda(const GLfloat sPlane[4], const GLfloat tPlane[4],
               GLfloat invQ, GLfloat width, GLfloat height)
{
   GLfloat dudx = sPlane[0] / sPlane[2] * invQ * width;
   GLfloat dudy = sPlane[1] / sPlane[2] * invQ * width;
   GLfloat dvdx = tPlane[0] / tPlane[2] * invQ * height;
   GLfloat dvdy = tPlane[1] / tPlane[2] * invQ * height;
   GLfloat r1 = dudx * dudx + dudy * dudy;
   GLfloat r2 = dvdx * dvdx + dvdy * dvdy;
   GLfloat rho2 = r1 + r2;
   /* return log base 2 of rho */
   return log(rho2) * 1.442695 * 0.5;       /* 1.442695 = 1/log(2) */
}

//#define DO_Z
//#define DO_RGBA
//#define DO_STUV0

static void
tex_aa_tri(GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
   const struct vertex_buffer *VB = ctx->VB;
   const GLfloat *p0 = VB->Win.data[v0];
   const GLfloat *p1 = VB->Win.data[v1];
   const GLfloat *p2 = VB->Win.data[v2];
   GLint vMin, vMid, vMax;
   GLint iyMin, iyMax;
   GLfloat yMin, yMax;
   GLboolean ltor;
   GLfloat majDx, majDy;
   GLfloat zPlane[4];                                       /* Z (depth) */
   GLdepth z[MAX_WIDTH];
   GLfloat rPlane[4], gPlane[4], bPlane[4], aPlane[4];      /* color */
   GLubyte rgba[MAX_WIDTH][4];
   GLfloat s0Plane[4], t0Plane[4], u0Plane[4], v0Plane[4];  /* texture 0 */
   GLfloat width0, height0;
   GLfloat s[2][MAX_WIDTH];
   GLfloat t[2][MAX_WIDTH];
   GLfloat u[2][MAX_WIDTH];
   GLfloat lambda[2][MAX_WIDTH];
   GLfloat bf = ctx->backface_sign;

   /* determine bottom to top order of vertices */
   {
      GLfloat y0 = VB->Win.data[v0][1];
      GLfloat y1 = VB->Win.data[v1][1];
      GLfloat y2 = VB->Win.data[v2][1];
      if (y0 <= y1) {
        if (y1 <= y2) {
           vMin = v0;   vMid = v1;   vMax = v2;   /* y0<=y1<=y2 */
        }
        else if (y2 <= y0) {
           vMin = v2;   vMid = v0;   vMax = v1;   /* y2<=y0<=y1 */
        }
        else {
           vMin = v0;   vMid = v2;   vMax = v1;  bf = -bf; /* y0<=y2<=y1 */
        }
      }
      else {
        if (y0 <= y2) {
           vMin = v1;   vMid = v0;   vMax = v2;  bf = -bf; /* y1<=y0<=y2 */
        }
        else if (y2 <= y1) {
           vMin = v2;   vMid = v1;   vMax = v0;  bf = -bf; /* y2<=y1<=y0 */
        }
        else {
           vMin = v1;   vMid = v2;   vMax = v0;   /* y1<=y2<=y0 */
        }
      }
   }

   majDx = VB->Win.data[vMax][0] - VB->Win.data[vMin][0];
   majDy = VB->Win.data[vMax][1] - VB->Win.data[vMin][1];

   {
      const GLfloat botDx = VB->Win.data[vMid][0] - VB->Win.data[vMin][0];
      const GLfloat botDy = VB->Win.data[vMid][1] - VB->Win.data[vMin][1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area * area < .0025)
        return;
   }

#ifndef DO_OCCLUSION_TEST
   ctx->OcclusionResult = GL_TRUE;
#endif

   /* plane setup */
   compute_plane(p0, p1, p2, p0[2], p1[2], p2[2], zPlane);
   if (ctx->Light.ShadeModel == GL_SMOOTH) {
      GLubyte (*rgba)[4] = VB->ColorPtr->data;
      compute_plane(p0, p1, p2, rgba[v0][0], rgba[v1][0], rgba[v2][0], rPlane);
      compute_plane(p0, p1, p2, rgba[v0][1], rgba[v1][1], rgba[v2][1], gPlane);
      compute_plane(p0, p1, p2, rgba[v0][2], rgba[v1][2], rgba[v2][2], bPlane);
      compute_plane(p0, p1, p2, rgba[v0][3], rgba[v1][3], rgba[v2][3], aPlane);
   }
   else {
      constant_plane(VB->ColorPtr->data[pv][0], rPlane);
      constant_plane(VB->ColorPtr->data[pv][1], gPlane);
      constant_plane(VB->ColorPtr->data[pv][2], bPlane);
      constant_plane(VB->ColorPtr->data[pv][3], aPlane);
   }
   {
      const struct gl_texture_object *obj = ctx->Texture.Unit[0].Current;
      const struct gl_texture_image *texImage = obj->Image[obj->BaseLevel];
      const GLint tSize = 3;
      const GLfloat invW0 = VB->Win.data[v0][3];
      const GLfloat invW1 = VB->Win.data[v1][3];
      const GLfloat invW2 = VB->Win.data[v2][3];
      GLfloat (*texCoord)[4] = VB->TexCoordPtr[0]->data;
      const GLfloat s0 = texCoord[v0][0] * invW0;
      const GLfloat s1 = texCoord[v1][0] * invW1;
      const GLfloat s2 = texCoord[v2][0] * invW2;
      const GLfloat t0 = (tSize > 1) ? texCoord[v0][1] * invW0 : 0.0F;
      const GLfloat t1 = (tSize > 1) ? texCoord[v1][1] * invW1 : 0.0F;
      const GLfloat t2 = (tSize > 1) ? texCoord[v2][1] * invW2 : 0.0F;
      const GLfloat r0 = (tSize > 2) ? texCoord[v0][2] * invW0 : 0.0F;
      const GLfloat r1 = (tSize > 2) ? texCoord[v1][2] * invW1 : 0.0F;
      const GLfloat r2 = (tSize > 2) ? texCoord[v2][2] * invW2 : 0.0F;
      const GLfloat q0 = (tSize > 3) ? texCoord[v0][3] * invW0 : invW0;
      const GLfloat q1 = (tSize > 3) ? texCoord[v1][3] * invW1 : invW1;
      const GLfloat q2 = (tSize > 3) ? texCoord[v2][3] * invW2 : invW2;
      compute_plane(p0, p1, p2, s0, s1, s2, s0Plane);
      compute_plane(p0, p1, p2, t0, t1, t2, t0Plane);
      compute_plane(p0, p1, p2, r0, r1, r2, u0Plane);
      compute_plane(p0, p1, p2, q0, q1, q2, v0Plane);
      width0 = (GLfloat) texImage->Width;
      height0 = (GLfloat) texImage->Height;
   }

   yMin = VB->Win.data[vMin][1];
   yMax = VB->Win.data[vMax][1];
   iyMin = (int) yMin;
   iyMax = (int) yMax + 1;

   if (ltor) {
      /* scan left to right */
      const float *pMin = VB->Win.data[vMin];
      const float *pMid = VB->Win.data[vMid];
      const float *pMax = VB->Win.data[vMax];
      const float dxdy = majDx / majDy;
      const float xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      float x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      int iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count, n;
         GLfloat coverage=0.f;
         /* skip over fragments with zero coverage */
         while (startX < MAX_WIDTH) {
            coverage = compute_coveragef(pMin, pMid, pMax, startX, iy);
            if (coverage > 0.0F)
               break;
            startX++;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            z[count] =(GLdepth) solve_plane(ix, iy, zPlane);
            rgba[count][RCOMP] = solve_plane_0_255(ix, iy, rPlane);
            rgba[count][GCOMP] = solve_plane_0_255(ix, iy, gPlane);
            rgba[count][BCOMP] = solve_plane_0_255(ix, iy, bPlane);
            rgba[count][ACOMP] = solve_plane_0_255(ix, iy, aPlane) * coverage;
            {
               GLfloat invQ = solve_plane_recip(ix, iy, v0Plane);
               s[0][count] = solve_plane(ix, iy, s0Plane) * invQ;
               t[0][count] = solve_plane(ix, iy, t0Plane) * invQ;
               u[0][count] = solve_plane(ix, iy, u0Plane) * invQ;
               lambda[0][count] = compute_lambda(s0Plane, t0Plane, invQ,
                                                 width0, height0);
            }
            ix++;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         n = (GLuint) ix - (GLuint) startX;
         gl_write_texture_span(ctx, n, startX, iy, z,
                               s[0], t[0], u[0], lambda[0],
                               rgba, 0, GL_POLYGON);
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = VB->Win.data[vMin];
      const GLfloat *pMid = VB->Win.data[vMid];
      const GLfloat *pMax = VB->Win.data[vMax];
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage=0.f;
         /* skip fragments with zero coverage */
         while (startX >= 0) {
            coverage = compute_coveragef(pMin, pMax, pMid, startX, iy);
            if (coverage > 0.0F)
               break;
            startX--;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            z[ix] = (GLdepth) solve_plane(ix, iy, zPlane);
            rgba[ix][0] = solve_plane_0_255(ix, iy, rPlane);
            rgba[ix][1] = solve_plane_0_255(ix, iy, gPlane);
            rgba[ix][2] = solve_plane_0_255(ix, iy, bPlane);
            rgba[ix][3] = solve_plane_0_255(ix, iy, aPlane) * coverage;
            {
               GLfloat invQ = solve_plane_recip(ix, iy, v0Plane);
               s[0][ix] = solve_plane(ix, iy, s0Plane) * invQ;
               t[0][ix] = solve_plane(ix, iy, t0Plane) * invQ;
               u[0][ix] = solve_plane(ix, iy, u0Plane) * invQ;
               lambda[0][ix] = compute_lambda(s0Plane, t0Plane, invQ,
                                              width0, height0);
            }
            ix--;
            count++;
            coverage = compute_coveragef(pMin, pMax, pMid, ix, iy);
         }

         n = (GLuint) startX - (GLuint) ix;
         left = ix + 1;
         gl_write_texture_span(ctx, n, left, iy, z + left,
                               s[0] + left, t[0] + left,
                               u[0] + left, lambda[0] + left,
                               rgba + left, 0, GL_POLYGON);
      }
   }
}

//#define DO_Z
//#define DO_RGBA
//#define DO_STUV0
//#define DO_SPEC

static void
spec_tex_aa_tri(GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{  const struct vertex_buffer *VB = ctx->VB;
   const GLfloat *p0 = VB->Win.data[v0];
   const GLfloat *p1 = VB->Win.data[v1];
   const GLfloat *p2 = VB->Win.data[v2];
   GLint vMin, vMid, vMax;
   GLint iyMin, iyMax;
   GLfloat yMin, yMax;
   GLboolean ltor;
   GLfloat majDx, majDy;
   GLfloat zPlane[4];                                       /* Z (depth) */
   GLdepth z[MAX_WIDTH];
   GLfloat rPlane[4], gPlane[4], bPlane[4], aPlane[4];      /* color */
   GLubyte rgba[MAX_WIDTH][4];
   GLfloat srPlane[4], sgPlane[4], sbPlane[4];              /* spec color */
   GLubyte spec[MAX_WIDTH][4];
   GLfloat s0Plane[4], t0Plane[4], u0Plane[4], v0Plane[4];  /* texture 0 */
   GLfloat width0, height0;
   GLfloat s[2][MAX_WIDTH];
   GLfloat t[2][MAX_WIDTH];
   GLfloat u[2][MAX_WIDTH];
   GLfloat lambda[2][MAX_WIDTH];
   GLfloat bf = ctx->backface_sign;

   /* determine bottom to top order of vertices */
   {
      GLfloat y0 = VB->Win.data[v0][1];
      GLfloat y1 = VB->Win.data[v1][1];
      GLfloat y2 = VB->Win.data[v2][1];
      if (y0 <= y1) {
        if (y1 <= y2) {
           vMin = v0;   vMid = v1;   vMax = v2;   /* y0<=y1<=y2 */
        }
        else if (y2 <= y0) {
           vMin = v2;   vMid = v0;   vMax = v1;   /* y2<=y0<=y1 */
        }
        else {
           vMin = v0;   vMid = v2;   vMax = v1;  bf = -bf; /* y0<=y2<=y1 */
        }
      }
      else {
        if (y0 <= y2) {
           vMin = v1;   vMid = v0;   vMax = v2;  bf = -bf; /* y1<=y0<=y2 */
        }
        else if (y2 <= y1) {
           vMin = v2;   vMid = v1;   vMax = v0;  bf = -bf; /* y2<=y1<=y0 */
        }
        else {
           vMin = v1;   vMid = v2;   vMax = v0;   /* y1<=y2<=y0 */
        }
      }
   }

   majDx = VB->Win.data[vMax][0] - VB->Win.data[vMin][0];
   majDy = VB->Win.data[vMax][1] - VB->Win.data[vMin][1];

   {
      const GLfloat botDx = VB->Win.data[vMid][0] - VB->Win.data[vMin][0];
      const GLfloat botDy = VB->Win.data[vMid][1] - VB->Win.data[vMin][1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area * area < .0025)
        return;
   }

#ifndef DO_OCCLUSION_TEST
   ctx->OcclusionResult = GL_TRUE;
#endif

   /* plane setup */
   compute_plane(p0, p1, p2, p0[2], p1[2], p2[2], zPlane);
   if (ctx->Light.ShadeModel == GL_SMOOTH) {
      GLubyte (*rgba)[4] = VB->ColorPtr->data;
      compute_plane(p0, p1, p2, rgba[v0][0], rgba[v1][0], rgba[v2][0], rPlane);
      compute_plane(p0, p1, p2, rgba[v0][1], rgba[v1][1], rgba[v2][1], gPlane);
      compute_plane(p0, p1, p2, rgba[v0][2], rgba[v1][2], rgba[v2][2], bPlane);
      compute_plane(p0, p1, p2, rgba[v0][3], rgba[v1][3], rgba[v2][3], aPlane);
   }
   else {
      constant_plane(VB->ColorPtr->data[pv][0], rPlane);
      constant_plane(VB->ColorPtr->data[pv][1], gPlane);
      constant_plane(VB->ColorPtr->data[pv][2], bPlane);
      constant_plane(VB->ColorPtr->data[pv][3], aPlane);
   }
   {
      GLubyte (*spec)[4] = VB->Specular;
      compute_plane(p0, p1, p2, spec[v0][0], spec[v1][0], spec[v2][0],srPlane);
      compute_plane(p0, p1, p2, spec[v0][1], spec[v1][1], spec[v2][1],sgPlane);
      compute_plane(p0, p1, p2, spec[v0][2], spec[v1][2], spec[v2][2],sbPlane);
   }
   {
      const struct gl_texture_object *obj = ctx->Texture.Unit[0].Current;
      const struct gl_texture_image *texImage = obj->Image[obj->BaseLevel];
      const GLint tSize = 3;
      const GLfloat invW0 = VB->Win.data[v0][3];
      const GLfloat invW1 = VB->Win.data[v1][3];
      const GLfloat invW2 = VB->Win.data[v2][3];
      GLfloat (*texCoord)[4] = VB->TexCoordPtr[0]->data;
      const GLfloat s0 = texCoord[v0][0] * invW0;
      const GLfloat s1 = texCoord[v1][0] * invW1;
      const GLfloat s2 = texCoord[v2][0] * invW2;
      const GLfloat t0 = (tSize > 1) ? texCoord[v0][1] * invW0 : 0.0F;
      const GLfloat t1 = (tSize > 1) ? texCoord[v1][1] * invW1 : 0.0F;
      const GLfloat t2 = (tSize > 1) ? texCoord[v2][1] * invW2 : 0.0F;
      const GLfloat r0 = (tSize > 2) ? texCoord[v0][2] * invW0 : 0.0F;
      const GLfloat r1 = (tSize > 2) ? texCoord[v1][2] * invW1 : 0.0F;
      const GLfloat r2 = (tSize > 2) ? texCoord[v2][2] * invW2 : 0.0F;
      const GLfloat q0 = (tSize > 3) ? texCoord[v0][3] * invW0 : invW0;
      const GLfloat q1 = (tSize > 3) ? texCoord[v1][3] * invW1 : invW1;
      const GLfloat q2 = (tSize > 3) ? texCoord[v2][3] * invW2 : invW2;
      compute_plane(p0, p1, p2, s0, s1, s2, s0Plane);
      compute_plane(p0, p1, p2, t0, t1, t2, t0Plane);
      compute_plane(p0, p1, p2, r0, r1, r2, u0Plane);
      compute_plane(p0, p1, p2, q0, q1, q2, v0Plane);
      width0 = (GLfloat) texImage->Width;
      height0 = (GLfloat) texImage->Height;
   }

   yMin = VB->Win.data[vMin][1];
   yMax = VB->Win.data[vMax][1];
   iyMin = (int) yMin;
   iyMax = (int) yMax + 1;

   if (ltor) {
      /* scan left to right */
      const float *pMin = VB->Win.data[vMin];
      const float *pMid = VB->Win.data[vMid];
      const float *pMax = VB->Win.data[vMax];
      const float dxdy = majDx / majDy;
      const float xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      float x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      int iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count, n;
         GLfloat coverage=0.f;
         /* skip over fragments with zero coverage */
         while (startX < MAX_WIDTH) {
            coverage = compute_coveragef(pMin, pMid, pMax, startX, iy);
            if (coverage > 0.0F)
               break;
            startX++;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            z[count] = (GLdepth) solve_plane(ix, iy, zPlane);
            rgba[count][0] = solve_plane_0_255(ix, iy, rPlane);
            rgba[count][1] = solve_plane_0_255(ix, iy, gPlane);
            rgba[count][2] = solve_plane_0_255(ix, iy, bPlane);
            rgba[count][3] = solve_plane_0_255(ix, iy, aPlane) * coverage;
            spec[count][0] = solve_plane_0_255(ix, iy, srPlane);
            spec[count][1] = solve_plane_0_255(ix, iy, sgPlane);
            spec[count][2] = solve_plane_0_255(ix, iy, sbPlane);
            {
               GLfloat invQ = solve_plane_recip(ix, iy, v0Plane);
               s[0][count] = solve_plane(ix, iy, s0Plane) * invQ;
               t[0][count] = solve_plane(ix, iy, t0Plane) * invQ;
               u[0][count] = solve_plane(ix, iy, u0Plane) * invQ;
               lambda[0][count] = compute_lambda(s0Plane, t0Plane, invQ,
                                                 width0, height0);
            }
            ix++;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         n = (GLuint) ix - (GLuint) startX;
         gl_write_texture_span(ctx, n, startX, iy, z,
                               s[0], t[0], u[0], lambda[0], rgba,
                               (const GLubyte (*)[4]) spec, GL_POLYGON);
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = VB->Win.data[vMin];
      const GLfloat *pMid = VB->Win.data[vMid];
      const GLfloat *pMax = VB->Win.data[vMax];
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage=0.f;
         /* skip fragments with zero coverage */
         while (startX >= 0) {
            coverage = compute_coveragef(pMin, pMax, pMid, startX, iy);
            if (coverage > 0.0F)
               break;
            startX--;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            z[ix] = (GLdepth) solve_plane(ix, iy, zPlane);
            rgba[ix][0] = solve_plane_0_255(ix, iy, rPlane);
            rgba[ix][1] = solve_plane_0_255(ix, iy, gPlane);
            rgba[ix][2] = solve_plane_0_255(ix, iy, bPlane);
            rgba[ix][3] = solve_plane_0_255(ix, iy, aPlane) * coverage;
            spec[ix][0] = solve_plane_0_255(ix, iy, srPlane);
            spec[ix][1] = solve_plane_0_255(ix, iy, sgPlane);
            spec[ix][2] = solve_plane_0_255(ix, iy, sbPlane);
            {
               GLfloat invQ = solve_plane_recip(ix, iy, v0Plane);
               s[0][ix] = solve_plane(ix, iy, s0Plane) * invQ;
               t[0][ix] = solve_plane(ix, iy, t0Plane) * invQ;
               u[0][ix] = solve_plane(ix, iy, u0Plane) * invQ;
               lambda[0][ix] = compute_lambda(s0Plane, t0Plane, invQ,
                                              width0, height0);
            }
            ix--;
            count++;
            coverage = compute_coveragef(pMin, pMax, pMid, ix, iy);
         }

         n = (GLuint) startX - (GLuint) ix;
         left = ix + 1;
         gl_write_texture_span(ctx, n, left, iy, z + left,
                               s[0] + left, t[0] + left, u[0] + left,
                               lambda[0] + left, rgba + left,
                               (const GLubyte (*)[4]) (spec + left),
                               GL_POLYGON);
      }
   }
}


//#define DO_Z
//#define DO_RGBA
//#define DO_STUV0
//#define DO_STUV1

static void
multitex_aa_tri(GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
   const struct vertex_buffer *VB = ctx->VB;
   const GLfloat *p0 = VB->Win.data[v0];
   const GLfloat *p1 = VB->Win.data[v1];
   const GLfloat *p2 = VB->Win.data[v2];
   GLint vMin, vMid, vMax;
   GLint iyMin, iyMax;
   GLfloat yMin, yMax;
   GLboolean ltor;
   GLfloat majDx, majDy;
   GLfloat zPlane[4];                                       /* Z (depth) */
   GLdepth z[MAX_WIDTH];
   GLfloat rPlane[4], gPlane[4], bPlane[4], aPlane[4];      /* color */
   GLubyte rgba[MAX_WIDTH][4];
   GLfloat s0Plane[4], t0Plane[4], u0Plane[4], v0Plane[4];  /* texture 0 */
   GLfloat width0, height0;
   GLfloat s[2][MAX_WIDTH];
   GLfloat t[2][MAX_WIDTH];
   GLfloat u[2][MAX_WIDTH];
   GLfloat lambda[2][MAX_WIDTH];
   GLfloat s1Plane[4], t1Plane[4], u1Plane[4], v1Plane[4];  /* texture 1 */
   GLfloat width1, height1;
   GLfloat bf = ctx->backface_sign;

   /* determine bottom to top order of vertices */
   {
      GLfloat y0 = VB->Win.data[v0][1];
      GLfloat y1 = VB->Win.data[v1][1];
      GLfloat y2 = VB->Win.data[v2][1];
      if (y0 <= y1) {
        if (y1 <= y2) {
           vMin = v0;   vMid = v1;   vMax = v2;   /* y0<=y1<=y2 */
        }
        else if (y2 <= y0) {
           vMin = v2;   vMid = v0;   vMax = v1;   /* y2<=y0<=y1 */
        }
        else {
           vMin = v0;   vMid = v2;   vMax = v1;  bf = -bf; /* y0<=y2<=y1 */
        }
      }
      else {
        if (y0 <= y2) {
           vMin = v1;   vMid = v0;   vMax = v2;  bf = -bf; /* y1<=y0<=y2 */
        }
        else if (y2 <= y1) {
           vMin = v2;   vMid = v1;   vMax = v0;  bf = -bf; /* y2<=y1<=y0 */
        }
        else {
           vMin = v1;   vMid = v2;   vMax = v0;   /* y1<=y2<=y0 */
        }
      }
   }

   majDx = VB->Win.data[vMax][0] - VB->Win.data[vMin][0];
   majDy = VB->Win.data[vMax][1] - VB->Win.data[vMin][1];

   {
      const GLfloat botDx = VB->Win.data[vMid][0] - VB->Win.data[vMin][0];
      const GLfloat botDy = VB->Win.data[vMid][1] - VB->Win.data[vMin][1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area * area < .0025)
        return;
   }

#ifndef DO_OCCLUSION_TEST
   ctx->OcclusionResult = GL_TRUE;
#endif

   /* plane setup */
   compute_plane(p0, p1, p2, p0[2], p1[2], p2[2], zPlane);
   if (ctx->Light.ShadeModel == GL_SMOOTH) {
      GLubyte (*rgba)[4] = VB->ColorPtr->data;
      compute_plane(p0, p1, p2, rgba[v0][0], rgba[v1][0], rgba[v2][0], rPlane);
      compute_plane(p0, p1, p2, rgba[v0][1], rgba[v1][1], rgba[v2][1], gPlane);
      compute_plane(p0, p1, p2, rgba[v0][2], rgba[v1][2], rgba[v2][2], bPlane);
      compute_plane(p0, p1, p2, rgba[v0][3], rgba[v1][3], rgba[v2][3], aPlane);
   }
   else {
      constant_plane(VB->ColorPtr->data[pv][0], rPlane);
      constant_plane(VB->ColorPtr->data[pv][1], gPlane);
      constant_plane(VB->ColorPtr->data[pv][2], bPlane);
      constant_plane(VB->ColorPtr->data[pv][3], aPlane);
   }
   {
      const struct gl_texture_object *obj = ctx->Texture.Unit[0].Current;
      const struct gl_texture_image *texImage = obj->Image[obj->BaseLevel];
      const GLint tSize = 3;
      const GLfloat invW0 = VB->Win.data[v0][3];
      const GLfloat invW1 = VB->Win.data[v1][3];
      const GLfloat invW2 = VB->Win.data[v2][3];
      GLfloat (*texCoord)[4] = VB->TexCoordPtr[0]->data;
      const GLfloat s0 = texCoord[v0][0] * invW0;
      const GLfloat s1 = texCoord[v1][0] * invW1;
      const GLfloat s2 = texCoord[v2][0] * invW2;
      const GLfloat t0 = (tSize > 1) ? texCoord[v0][1] * invW0 : 0.0F;
      const GLfloat t1 = (tSize > 1) ? texCoord[v1][1] * invW1 : 0.0F;
      const GLfloat t2 = (tSize > 1) ? texCoord[v2][1] * invW2 : 0.0F;
      const GLfloat r0 = (tSize > 2) ? texCoord[v0][2] * invW0 : 0.0F;
      const GLfloat r1 = (tSize > 2) ? texCoord[v1][2] * invW1 : 0.0F;
      const GLfloat r2 = (tSize > 2) ? texCoord[v2][2] * invW2 : 0.0F;
      const GLfloat q0 = (tSize > 3) ? texCoord[v0][3] * invW0 : invW0;
      const GLfloat q1 = (tSize > 3) ? texCoord[v1][3] * invW1 : invW1;
      const GLfloat q2 = (tSize > 3) ? texCoord[v2][3] * invW2 : invW2;
      compute_plane(p0, p1, p2, s0, s1, s2, s0Plane);
      compute_plane(p0, p1, p2, t0, t1, t2, t0Plane);
      compute_plane(p0, p1, p2, r0, r1, r2, u0Plane);
      compute_plane(p0, p1, p2, q0, q1, q2, v0Plane);
      width0 = (GLfloat) texImage->Width;
      height0 = (GLfloat) texImage->Height;
   }
   {
      const struct gl_texture_object *obj = ctx->Texture.Unit[1].Current;
      const struct gl_texture_image *texImage = obj->Image[obj->BaseLevel];
      const GLint tSize = VB->TexCoordPtr[1]->size;
      const GLfloat invW0 = VB->Win.data[v0][3];
      const GLfloat invW1 = VB->Win.data[v1][3];
      const GLfloat invW2 = VB->Win.data[v2][3];
      GLfloat (*texCoord)[4] = VB->TexCoordPtr[1]->data;
      const GLfloat s0 = texCoord[v0][0] * invW0;
      const GLfloat s1 = texCoord[v1][0] * invW1;
      const GLfloat s2 = texCoord[v2][0] * invW2;
      const GLfloat t0 = (tSize > 1) ? texCoord[v0][1] * invW0 : 0.0F;
      const GLfloat t1 = (tSize > 1) ? texCoord[v1][1] * invW1 : 0.0F;
      const GLfloat t2 = (tSize > 1) ? texCoord[v2][1] * invW2 : 0.0F;
      const GLfloat r0 = (tSize > 2) ? texCoord[v0][2] * invW0 : 0.0F;
      const GLfloat r1 = (tSize > 2) ? texCoord[v1][2] * invW1 : 0.0F;
      const GLfloat r2 = (tSize > 2) ? texCoord[v2][2] * invW2 : 0.0F;
      const GLfloat q0 = (tSize > 3) ? texCoord[v0][3] * invW0 : invW0;
      const GLfloat q1 = (tSize > 3) ? texCoord[v1][3] * invW1 : invW1;
      const GLfloat q2 = (tSize > 3) ? texCoord[v2][3] * invW2 : invW2;
      compute_plane(p0, p1, p2, s0, s1, s2, s1Plane);
      compute_plane(p0, p1, p2, t0, t1, t2, t1Plane);
      compute_plane(p0, p1, p2, r0, r1, r2, u1Plane);
      compute_plane(p0, p1, p2, q0, q1, q2, v1Plane);
      width1 = (GLfloat) texImage->Width;
      height1 = (GLfloat) texImage->Height;
   }

   yMin = VB->Win.data[vMin][1];
   yMax = VB->Win.data[vMax][1];
   iyMin = (int) yMin;
   iyMax = (int) yMax + 1;

   if (ltor) {
      /* scan left to right */
      const float *pMin = VB->Win.data[vMin];
      const float *pMid = VB->Win.data[vMid];
      const float *pMax = VB->Win.data[vMax];
      const float dxdy = majDx / majDy;
      const float xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      float x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      int iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count, n;
         GLfloat coverage=0.f;
         /* skip over fragments with zero coverage */
         while (startX < MAX_WIDTH) {
            coverage = compute_coveragef(pMin, pMid, pMax, startX, iy);
            if (coverage > 0.0F)
               break;
            startX++;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            z[count] = (GLdepth) solve_plane(ix, iy, zPlane);
            rgba[count][0] = solve_plane_0_255(ix, iy, rPlane);
            rgba[count][1] = solve_plane_0_255(ix, iy, gPlane);
            rgba[count][2] = solve_plane_0_255(ix, iy, bPlane);
            rgba[count][3] = solve_plane_0_255(ix, iy, aPlane) * coverage;
            {
               GLfloat invQ = solve_plane_recip(ix, iy, v0Plane);
               s[0][count] = solve_plane(ix, iy, s0Plane) * invQ;
               t[0][count] = solve_plane(ix, iy, t0Plane) * invQ;
               u[0][count] = solve_plane(ix, iy, u0Plane) * invQ;
               lambda[0][count] = compute_lambda(s0Plane, t0Plane, invQ,
                                                 width0, height0);
            }
            {
               GLfloat invQ = solve_plane_recip(ix, iy, v1Plane);
               s[1][count] = solve_plane(ix, iy, s1Plane) * invQ;
               t[1][count] = solve_plane(ix, iy, t1Plane) * invQ;
               u[1][count] = solve_plane(ix, iy, u1Plane) * invQ;
               lambda[1][count] = compute_lambda(s1Plane, t1Plane, invQ,
                                                 width1, height1);
            }
            ix++;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         n = (GLuint) ix - (GLuint) startX;
         gl_write_multitexture_span(ctx, 2, n, startX, iy, z,
                                    (const GLfloat (*)[MAX_WIDTH]) s,
                                    (const GLfloat (*)[MAX_WIDTH]) t,
                                    (const GLfloat (*)[MAX_WIDTH]) u,
                                    lambda, rgba, 0, GL_POLYGON);
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = VB->Win.data[vMin];
      const GLfloat *pMid = VB->Win.data[vMid];
      const GLfloat *pMax = VB->Win.data[vMax];
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage=0.f;
         /* skip fragments with zero coverage */
         while (startX >= 0) {
            coverage = compute_coveragef(pMin, pMax, pMid, startX, iy);
            if (coverage > 0.0F)
               break;
            startX--;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            z[ix] = (GLdepth) solve_plane(ix, iy, zPlane);
            rgba[ix][0] = solve_plane_0_255(ix, iy, rPlane);
            rgba[ix][1] = solve_plane_0_255(ix, iy, gPlane);
            rgba[ix][2] = solve_plane_0_255(ix, iy, bPlane);
            rgba[ix][3] = solve_plane_0_255(ix, iy, aPlane) * coverage;
            {
               GLfloat invQ = solve_plane_recip(ix, iy, v0Plane);
               s[0][ix] = solve_plane(ix, iy, s0Plane) * invQ;
               t[0][ix] = solve_plane(ix, iy, t0Plane) * invQ;
               u[0][ix] = solve_plane(ix, iy, u0Plane) * invQ;
               lambda[0][ix] = compute_lambda(s0Plane, t0Plane, invQ,
                                              width0, height0);
            }
            {
               GLfloat invQ = solve_plane_recip(ix, iy, v1Plane);
               s[1][ix] = solve_plane(ix, iy, s1Plane) * invQ;
               t[1][ix] = solve_plane(ix, iy, t1Plane) * invQ;
               u[1][ix] = solve_plane(ix, iy, u1Plane) * invQ;
               lambda[1][ix] = compute_lambda(s1Plane, t1Plane, invQ,
                                              width1, height1);
            }
            ix--;
            count++;
            coverage = compute_coveragef(pMin, pMax, pMid, ix, iy);
         }

         n = (GLuint) startX - (GLuint) ix;
         left = ix + 1;
         {
            int j;
            for (j = 0; j < n; j++) {
               s[0][j] = s[0][j + left];
               t[0][j] = t[0][j + left];
               u[0][j] = u[0][j + left];
               s[1][j] = s[1][j + left];
               t[1][j] = t[1][j + left];
               u[1][j] = u[1][j + left];
               lambda[0][j] = lambda[0][j + left];
               lambda[1][j] = lambda[1][j + left];
            }
         }
         gl_write_multitexture_span(ctx, 2, n, left, iy, z + left,
                                    (const GLfloat (*)[MAX_WIDTH]) s,
                                    (const GLfloat (*)[MAX_WIDTH]) t,
                                    (const GLfloat (*)[MAX_WIDTH]) u,
                                    lambda,
                                    rgba + left, 0, GL_POLYGON);
      }
   }
}


//#define DO_Z       compute Z values
//#define DO_RGBA    compute RGBA values
//#define DO_STUV0   compute unit 0 STRQ texcoords
//#define DO_STUV1   compute unit 1 STRQ texcoords
//#define DO_SPEC    compute specular RGB values


static void
spec_multitex_aa_tri(GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv)
{
   const struct vertex_buffer *VB = ctx->VB;
   const GLfloat *p0 = VB->Win.data[v0];
   const GLfloat *p1 = VB->Win.data[v1];
   const GLfloat *p2 = VB->Win.data[v2];
   GLint vMin, vMid, vMax;
   GLint iyMin, iyMax;
   GLfloat yMin, yMax;
   GLboolean ltor;
   GLfloat majDx, majDy;
   GLfloat zPlane[4];                                       /* Z (depth) */
   GLdepth z[MAX_WIDTH];
   GLfloat rPlane[4], gPlane[4], bPlane[4], aPlane[4];      /* color */
   GLubyte rgba[MAX_WIDTH][4];
   GLfloat srPlane[4], sgPlane[4], sbPlane[4];              /* spec color */
   GLubyte spec[MAX_WIDTH][4];
   GLfloat s0Plane[4], t0Plane[4], u0Plane[4], v0Plane[4];  /* texture 0 */
   GLfloat width0, height0;
   GLfloat s[2][MAX_WIDTH];
   GLfloat t[2][MAX_WIDTH];
   GLfloat u[2][MAX_WIDTH];
   GLfloat lambda[2][MAX_WIDTH];
   GLfloat s1Plane[4], t1Plane[4], u1Plane[4], v1Plane[4];  /* texture 1 */
   GLfloat width1, height1;
   GLfloat bf = ctx->backface_sign;

   /* determine bottom to top order of vertices */
   {
      GLfloat y0 = VB->Win.data[v0][1];
      GLfloat y1 = VB->Win.data[v1][1];
      GLfloat y2 = VB->Win.data[v2][1];
      if (y0 <= y1) {
        if (y1 <= y2) {
           vMin = v0;   vMid = v1;   vMax = v2;   /* y0<=y1<=y2 */
        }
        else if (y2 <= y0) {
           vMin = v2;   vMid = v0;   vMax = v1;   /* y2<=y0<=y1 */
        }
        else {
           vMin = v0;   vMid = v2;   vMax = v1;  bf = -bf; /* y0<=y2<=y1 */
        }
      }
      else {
        if (y0 <= y2) {
           vMin = v1;   vMid = v0;   vMax = v2;  bf = -bf; /* y1<=y0<=y2 */
        }
        else if (y2 <= y1) {
           vMin = v2;   vMid = v1;   vMax = v0;  bf = -bf; /* y2<=y1<=y0 */
        }
        else {
           vMin = v1;   vMid = v2;   vMax = v0;   /* y1<=y2<=y0 */
        }
      }
   }

   majDx = VB->Win.data[vMax][0] - VB->Win.data[vMin][0];
   majDy = VB->Win.data[vMax][1] - VB->Win.data[vMin][1];

   {
      const GLfloat botDx = VB->Win.data[vMid][0] - VB->Win.data[vMin][0];
      const GLfloat botDy = VB->Win.data[vMid][1] - VB->Win.data[vMin][1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area * area < .0025)
        return;
   }

#ifndef DO_OCCLUSION_TEST
   ctx->OcclusionResult = GL_TRUE;
#endif

   /* plane setup */
   compute_plane(p0, p1, p2, p0[2], p1[2], p2[2], zPlane);
   if (ctx->Light.ShadeModel == GL_SMOOTH) {
      GLubyte (*rgba)[4] = VB->ColorPtr->data;
      compute_plane(p0, p1, p2, rgba[v0][0], rgba[v1][0], rgba[v2][0], rPlane);
      compute_plane(p0, p1, p2, rgba[v0][1], rgba[v1][1], rgba[v2][1], gPlane);
      compute_plane(p0, p1, p2, rgba[v0][2], rgba[v1][2], rgba[v2][2], bPlane);
      compute_plane(p0, p1, p2, rgba[v0][3], rgba[v1][3], rgba[v2][3], aPlane);
   }
   else {
      constant_plane(VB->ColorPtr->data[pv][0], rPlane);
      constant_plane(VB->ColorPtr->data[pv][1], gPlane);
      constant_plane(VB->ColorPtr->data[pv][2], bPlane);
      constant_plane(VB->ColorPtr->data[pv][3], aPlane);
   }
   {
      GLubyte (*spec)[4] = VB->Specular;
      compute_plane(p0, p1, p2, spec[v0][0], spec[v1][0], spec[v2][0],srPlane);
      compute_plane(p0, p1, p2, spec[v0][1], spec[v1][1], spec[v2][1],sgPlane);
      compute_plane(p0, p1, p2, spec[v0][2], spec[v1][2], spec[v2][2],sbPlane);
   }
   {
      const struct gl_texture_object *obj = ctx->Texture.Unit[0].Current;
      const struct gl_texture_image *texImage = obj->Image[obj->BaseLevel];
      const GLint tSize = 3;
      const GLfloat invW0 = VB->Win.data[v0][3];
      const GLfloat invW1 = VB->Win.data[v1][3];
      const GLfloat invW2 = VB->Win.data[v2][3];
      GLfloat (*texCoord)[4] = VB->TexCoordPtr[0]->data;
      const GLfloat s0 = texCoord[v0][0] * invW0;
      const GLfloat s1 = texCoord[v1][0] * invW1;
      const GLfloat s2 = texCoord[v2][0] * invW2;
      const GLfloat t0 = (tSize > 1) ? texCoord[v0][1] * invW0 : 0.0F;
      const GLfloat t1 = (tSize > 1) ? texCoord[v1][1] * invW1 : 0.0F;
      const GLfloat t2 = (tSize > 1) ? texCoord[v2][1] * invW2 : 0.0F;
      const GLfloat r0 = (tSize > 2) ? texCoord[v0][2] * invW0 : 0.0F;
      const GLfloat r1 = (tSize > 2) ? texCoord[v1][2] * invW1 : 0.0F;
      const GLfloat r2 = (tSize > 2) ? texCoord[v2][2] * invW2 : 0.0F;
      const GLfloat q0 = (tSize > 3) ? texCoord[v0][3] * invW0 : invW0;
      const GLfloat q1 = (tSize > 3) ? texCoord[v1][3] * invW1 : invW1;
      const GLfloat q2 = (tSize > 3) ? texCoord[v2][3] * invW2 : invW2;
      compute_plane(p0, p1, p2, s0, s1, s2, s0Plane);
      compute_plane(p0, p1, p2, t0, t1, t2, t0Plane);
      compute_plane(p0, p1, p2, r0, r1, r2, u0Plane);
      compute_plane(p0, p1, p2, q0, q1, q2, v0Plane);
      width0 = (GLfloat) texImage->Width;
      height0 = (GLfloat) texImage->Height;
   }
   {
      const struct gl_texture_object *obj = ctx->Texture.Unit[1].Current;
      const struct gl_texture_image *texImage = obj->Image[obj->BaseLevel];
      const GLint tSize = VB->TexCoordPtr[1]->size;
      const GLfloat invW0 = VB->Win.data[v0][3];
      const GLfloat invW1 = VB->Win.data[v1][3];
      const GLfloat invW2 = VB->Win.data[v2][3];
      GLfloat (*texCoord)[4] = VB->TexCoordPtr[1]->data;
      const GLfloat s0 = texCoord[v0][0] * invW0;
      const GLfloat s1 = texCoord[v1][0] * invW1;
      const GLfloat s2 = texCoord[v2][0] * invW2;
      const GLfloat t0 = (tSize > 1) ? texCoord[v0][1] * invW0 : 0.0F;
      const GLfloat t1 = (tSize > 1) ? texCoord[v1][1] * invW1 : 0.0F;
      const GLfloat t2 = (tSize > 1) ? texCoord[v2][1] * invW2 : 0.0F;
      const GLfloat r0 = (tSize > 2) ? texCoord[v0][2] * invW0 : 0.0F;
      const GLfloat r1 = (tSize > 2) ? texCoord[v1][2] * invW1 : 0.0F;
      const GLfloat r2 = (tSize > 2) ? texCoord[v2][2] * invW2 : 0.0F;
      const GLfloat q0 = (tSize > 3) ? texCoord[v0][3] * invW0 : invW0;
      const GLfloat q1 = (tSize > 3) ? texCoord[v1][3] * invW1 : invW1;
      const GLfloat q2 = (tSize > 3) ? texCoord[v2][3] * invW2 : invW2;
      compute_plane(p0, p1, p2, s0, s1, s2, s1Plane);
      compute_plane(p0, p1, p2, t0, t1, t2, t1Plane);
      compute_plane(p0, p1, p2, r0, r1, r2, u1Plane);
      compute_plane(p0, p1, p2, q0, q1, q2, v1Plane);
      width1 = (GLfloat) texImage->Width;
      height1 = (GLfloat) texImage->Height;
   }

   yMin = VB->Win.data[vMin][1];
   yMax = VB->Win.data[vMax][1];
   iyMin = (int) yMin;
   iyMax = (int) yMax + 1;

   if (ltor) {
      /* scan left to right */
      const float *pMin = VB->Win.data[vMin];
      const float *pMid = VB->Win.data[vMid];
      const float *pMax = VB->Win.data[vMax];
      const float dxdy = majDx / majDy;
      const float xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      float x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      int iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count, n;
         GLfloat coverage=0.f;
         /* skip over fragments with zero coverage */
         while (startX < MAX_WIDTH) {
            coverage = compute_coveragef(pMin, pMid, pMax, startX, iy);
            if (coverage > 0.0F)
               break;
            startX++;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            z[count] = (GLdepth) solve_plane(ix, iy, zPlane);
            rgba[count][0] = solve_plane_0_255(ix, iy, rPlane);
            rgba[count][1] = solve_plane_0_255(ix, iy, gPlane);
            rgba[count][2] = solve_plane_0_255(ix, iy, bPlane);
            rgba[count][3] = solve_plane_0_255(ix, iy, aPlane) * coverage;
            spec[count][0] = solve_plane_0_255(ix, iy, srPlane);
            spec[count][1] = solve_plane_0_255(ix, iy, sgPlane);
            spec[count][2] = solve_plane_0_255(ix, iy, sbPlane);
            {
               GLfloat invQ = solve_plane_recip(ix, iy, v0Plane);
               s[0][count] = solve_plane(ix, iy, s0Plane) * invQ;
               t[0][count] = solve_plane(ix, iy, t0Plane) * invQ;
               u[0][count] = solve_plane(ix, iy, u0Plane) * invQ;
               lambda[0][count] = compute_lambda(s0Plane, t0Plane, invQ,
                                                 width0, height0);
            }
            {
               GLfloat invQ = solve_plane_recip(ix, iy, v1Plane);
               s[1][count] = solve_plane(ix, iy, s1Plane) * invQ;
               t[1][count] = solve_plane(ix, iy, t1Plane) * invQ;
               u[1][count] = solve_plane(ix, iy, u1Plane) * invQ;
               lambda[1][count] = compute_lambda(s1Plane, t1Plane, invQ,
                                                 width1, height1);
            }
            ix++;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         n = (GLuint) ix - (GLuint) startX;
         gl_write_multitexture_span(ctx, 2, n, startX, iy, z,
                                    (const GLfloat (*)[MAX_WIDTH]) s,
                                    (const GLfloat (*)[MAX_WIDTH]) t,
                                    (const GLfloat (*)[MAX_WIDTH]) u,
                                    (GLfloat (*)[MAX_WIDTH]) lambda,
                                    rgba, (const GLubyte (*)[4]) spec,
                                    GL_POLYGON);
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = VB->Win.data[vMin];
      const GLfloat *pMid = VB->Win.data[vMid];
      const GLfloat *pMax = VB->Win.data[vMax];
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = VB->Win.data[vMin][0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage=0.f;
         /* skip fragments with zero coverage */
         while (startX >= 0) {
            coverage = compute_coveragef(pMin, pMax, pMid, startX, iy);
            if (coverage > 0.0F)
               break;
            startX--;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            z[ix] = (GLdepth) solve_plane(ix, iy, zPlane);
            rgba[ix][0] = solve_plane_0_255(ix, iy, rPlane);
            rgba[ix][1] = solve_plane_0_255(ix, iy, gPlane);
            rgba[ix][2] = solve_plane_0_255(ix, iy, bPlane);
            rgba[ix][3] = solve_plane_0_255(ix, iy, aPlane) * coverage;
            spec[ix][0] = solve_plane_0_255(ix, iy, srPlane);
            spec[ix][1] = solve_plane_0_255(ix, iy, sgPlane);
            spec[ix][2] = solve_plane_0_255(ix, iy, sbPlane);
            {
               GLfloat invQ = solve_plane_recip(ix, iy, v0Plane);
               s[0][ix] = solve_plane(ix, iy, s0Plane) * invQ;
               t[0][ix] = solve_plane(ix, iy, t0Plane) * invQ;
               u[0][ix] = solve_plane(ix, iy, u0Plane) * invQ;
               lambda[0][ix] = compute_lambda(s0Plane, t0Plane, invQ,
                                              width0, height0);
            }
            {
               GLfloat invQ = solve_plane_recip(ix, iy, v1Plane);
               s[1][ix] = solve_plane(ix, iy, s1Plane) * invQ;
               t[1][ix] = solve_plane(ix, iy, t1Plane) * invQ;
               u[1][ix] = solve_plane(ix, iy, u1Plane) * invQ;
               lambda[1][ix] = compute_lambda(s1Plane, t1Plane, invQ,
                                              width1, height1);
            }
            ix--;
            count++;
            coverage = compute_coveragef(pMin, pMax, pMid, ix, iy);
         }

         n = (GLuint) startX - (GLuint) ix;
         left = ix + 1;
         {
            int j;
            for (j = 0; j < n; j++) {
               s[0][j] = s[0][j + left];
               t[0][j] = t[0][j + left];
               u[0][j] = u[0][j + left];
               s[1][j] = s[1][j + left];
               t[1][j] = t[1][j + left];
               u[1][j] = u[1][j + left];
               lambda[0][j] = lambda[0][j + left];
               lambda[1][j] = lambda[1][j + left];
            }
         }
         gl_write_multitexture_span(ctx, 2, n, left, iy, z + left,
                                    (const GLfloat (*)[MAX_WIDTH]) s,
                                    (const GLfloat (*)[MAX_WIDTH]) t,
                                    (const GLfloat (*)[MAX_WIDTH]) u,
                                    lambda, rgba + left,
                                    (const GLubyte (*)[4]) (spec + left),
                                    GL_POLYGON);
      }
   }
}



/*
 * Examine GL state and set ctx->Driver.TriangleFunc to an
 * appropriate antialiased triangle rasterizer function.
 */
void
_mesa_set_aa_triangle_function(GLcontext *ctx)
{
   ;
   if (ctx->Texture.ReallyEnabled) {
      if (ctx->Light.Enabled &&
          ctx->Light.Model.ColorControl==0x81FA) {
         if (ctx->Texture.ReallyEnabled >= (1<<4)) {
            ctx->Driver.TriangleFunc = spec_multitex_aa_tri;
         }
         else {
            ctx->Driver.TriangleFunc = spec_tex_aa_tri;
         }
      }
      else {
         if (ctx->Texture.ReallyEnabled >= (1<<4)) {
            ctx->Driver.TriangleFunc = multitex_aa_tri;
         }
         else {
            ctx->Driver.TriangleFunc = tex_aa_tri;
         }
      }
   }
   else {
      if (ctx->Visual->RGBAflag) {
         ctx->Driver.TriangleFunc = rgba_aa_tri;
      }
      else {
         ctx->Driver.TriangleFunc = index_aa_tri;
      }
   }
   ;
}
