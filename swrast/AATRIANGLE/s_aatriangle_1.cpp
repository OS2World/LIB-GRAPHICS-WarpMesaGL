/* $Id: s_aatriangle_1.c,v 1.26 2002/10/24 23:57:24 brianp Exp $ */
#include "glheader.h"
#include "macros.h"
#include "imports.h"
#include "mmath.h"
#include "s_aatriangle.h"
#include "s_context.h"
#include "s_span.h"

/****************************/
#define FIST_MAGIC ((((65536.0 * 65536.0 * 16)+(65536.0 * 0.5))* 65536.0))

/*
 * Compute coefficients of a plane using the X,Y coords of the v0, v1, v2
 * vertices and the given Z values.
 * A point (x,y,z) lies on plane iff a*x+b*y+c*z+d = 0.
 */
static   void INLINE
compute_plane(const GLfloat v0[], const GLfloat v1[], const GLfloat v2[],
              GLfloat z0, GLfloat z1, GLfloat z2, GLfloat plane[4])
{
   const GLfloat px = v1[0] - v0[0];
   const GLfloat py = v1[1] - v0[1];
   const GLfloat pz = z1 - z0;

   const GLfloat qx = v2[0] - v0[0];
   const GLfloat qy = v2[1] - v0[1];
   const GLfloat qz = z2 - z0;

   /* Crossproduct "(a,b,c):= dv1 x dv2" is orthogonal to plane. */
   const GLfloat a = py * qz - pz * qy;
   const GLfloat b = pz * qx - px * qz;
   const GLfloat c = px * qy - py * qx;
   /* Point on the plane = "r*(a,b,c) + w", with fixed "r" depending
      on the distance of plane from origin and arbitrary "w" parallel
      to the plane. */
   /* The scalar product "(r*(a,b,c)+w)*(a,b,c)" is "r*(a^2+b^2+c^2)",
      which is equal to "-d" below. */
   const GLfloat d = -(a * v0[0] + b * v0[1] + c * z0);

   plane[0] = a;
   plane[1] = b;
   plane[2] = c;
   plane[3] = d;
}


/*
 * Compute coefficients of a plane with a constant Z value.
 */
static void INLINE
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
static  GLfloat INLINE
solve_plane(GLfloat x, GLfloat y, const GLfloat plane[4])
{
   return (plane[3] + plane[0] * x + plane[1] * y) / -plane[2];
}

/*
 * Solve plane and return clamped GLchan value.
 */
static  GLchan INLINE
solve_plane_chan(GLfloat x, GLfloat y, const GLfloat plane[4])
{
   GLfloat z = (plane[3] + plane[0] * x + plane[1] * y) / -plane[2] + 0.5F;
   if (z < 0.0F)
      return 0;
   else if (z > 255.0F)
      return (GLchan) 255.0F;
   return (GLchan) (GLint) z;
}

static  GLfloat
AAA_compute_coveragef(const GLfloat v0[3], const GLfloat v1[3],
                  const GLfloat v2[3], GLint winx, GLint winy)
{
   static const GLfloat samples[16][2] = {
      /* start with the four corners */
      { (0.5+0*4+2)/16, (0.5+0*4+0)/16 },
      { (0.5+3*4+3)/16, (0.5+0*4+2)/16 },
      { (0.5+0*4+0)/16, (0.5+3*4+1)/16 },
      { (0.5+3*4+1)/16, (0.5+3*4+3)/16 },
      /* continue with interior samples */
      { (0.5+1*4+1)/16, (0.5+0*4+1)/16 },
      { (0.5+2*4+0)/16, (0.5+0*4+3)/16 },
      { (0.5+0*4+3)/16, (0.5+1*4+3)/16 },
      { (0.5+1*4+2)/16, (0.5+1*4+0)/16 },
      { (0.5+2*4+3)/16, (0.5+1*4+2)/16 },
      { (0.5+3*4+2)/16, (0.5+1*4+1)/16 },
      { (0.5+0*4+1)/16, (0.5+2*4+2)/16 },
      { (0.5+1*4+0)/16, (0.5+2*4+1)/16 },
      { (0.5+2*4+1)/16, (0.5+2*4+3)/16 },
      { (0.5+3*4+0)/16, (0.5+2*4+0)/16 },
      { (0.5+1*4+3)/16, (0.5+3*4+0)/16 },
      { (0.5+2*4+2)/16, (0.5+3*4+2)/16 }
   };

   const GLfloat x = (GLfloat) winx;
   const GLfloat y = (GLfloat) winy;
   const GLfloat dx0 = v1[0] - v0[0];
   const GLfloat dy0 = v1[1] - v0[1];
   const GLfloat dx1 = v2[0] - v1[0];
   const GLfloat dy1 = v2[1] - v1[1];
/* const */ GLfloat dx2 = v0[0] - v2[0];
/* const */ GLfloat dy2 = v0[1] - v2[1];
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
/**********************************************/
/*
 * Compute how much (area) of the given pixel is inside the triangle.
 * Vertices MUST be specified in counter-clockwise order.
 * Return:  coverage in [0, 1].
 */
static GLfloat
compute_coveragef(const GLfloat v0[3], const GLfloat v1[3],
                  const GLfloat v2[3], GLint winx, GLint winy)
{
   /* Given a position [0,3]x[0,3] return the sub-pixel sample position.
    * Contributed by Ray Tice.
    *
    * Jitter sample positions -
    * - average should be .5 in x & y for each column
    * - each of the 16 rows and columns should be used once
    * - the rectangle formed by the first four points
    *   should contain the other points
    * - the distrubition should be fairly even in any given direction
    *
    * The pattern drawn below isn't optimal, but it's better than a regular
    * grid.  In the drawing, the center of each subpixel is surrounded by
    * four dots.  The "x" marks the jittered position relative to the
    * subpixel center.
    */
#define POS(a, b) (0.5+a*4+b)/16
   static const GLfloat samples[16][2] = {
      /* start with the four corners */
      { POS(0, 2), POS(0, 0) },
      { POS(3, 3), POS(0, 2) },
      { POS(0, 0), POS(3, 1) },
      { POS(3, 1), POS(3, 3) },
      /* continue with interior samples */
      { POS(1, 1), POS(0, 1) },
      { POS(2, 0), POS(0, 3) },
      { POS(0, 3), POS(1, 3) },
      { POS(1, 2), POS(1, 0) },
      { POS(2, 3), POS(1, 2) },
      { POS(3, 2), POS(1, 1) },
      { POS(0, 1), POS(2, 2) },
      { POS(1, 0), POS(2, 1) },
      { POS(2, 1), POS(2, 3) },
      { POS(3, 0), POS(2, 0) },
      { POS(1, 3), POS(3, 0) },
      { POS(2, 2), POS(3, 2) }
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
   GLfloat insideCount = 16.0F;

#ifdef DEBUG
   {
      const GLfloat area = dx0 * dy1 - dx1 * dy0;
      ASSERT(area >= 0.0);
   }
#endif

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
         insideCount -= 1.0F;
         stop = 16;
      }
   }
   if (stop == 4)
      return 1.0F;
   else
      return insideCount * (1.0F / 16.0F);
}


/**********************************************/
//EK DEBUG
int debug_write(int *buff, int n);

extern void
_mesa_debug( const __GLcontext *ctx, const char *fmtString, ... );

/* static */
//void
//rgba_aa_tri(GLcontext *ctx,
//           const SWvertex *v0,
//           const SWvertex *v1,
//           const SWvertex *v2)
F_swrast_tri_func rgba_aa_tri;

void
rgba_aa_tri(GLcontext *ctx,
           const SWvertex *v0,
           const SWvertex *v1,
           const SWvertex *v2)

/*void triangle( GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv )*/
{
     const GLfloat *p0 = v0->win;
     const GLfloat *p1 = v1->win;
     const GLfloat *p2 = v2->win;
     const SWvertex *vMin, *vMid, *vMax;
     GLint   iyMin, iyMax, ixMin,ixMax;
     GLfloat yMin,  yMax,  xMax, xMin;
     GLboolean ltor;
     GLfloat majDx, majDy;  /* major (i.e. long) edge dx and dy */
     struct sw_span span;
     GLint iy;
     GLint ix, left;
     GLint j,jj;
     GLint  startX;
     GLuint count, n;
     GLfloat x;
     GLfloat zPlane[4];
     GLfloat fogPlane[4];
     GLfloat rPlane[4], gPlane[4], bPlane[4], aPlane[4];
     GLfloat coverage;
/* static */   GLfloat bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;

// {
//      _mesa_debug(ctx, "rgba_aa_tri_start: v0=%p,v1=%p, v2=%p\n",v0,v1,v2);
//      _mesa_debug(ctx, "(%f,%f,%f),(%f,%f,%f),(%f,%f,%f)\n",
//        v0->win[0],v0->win[1],v0->win[2],
//        v1->win[0],v1->win[1],v1->win[2],
//        v2->win[0],v2->win[1],v2->win[2] );
//
// }
//printf(__FUNCTION__"ctx=%p,pv1=%p,pv2=%p,v3=%p",   ctx, v0,v1,v2);
//            printf(",v1=%i,v2=%i,3=%i\n",  *v0,*v1,*v2);
//  { int tmp[4];
//    tmp[0] = 0;
//    if(ctx) tmp[0] = 1;
//    tmp[1] = 0;
//    if(v0) tmp[1] = 1;
//    tmp[2] = 0;
//    if(v1) tmp[2] = 1;
//    tmp[3] = 0;
//    if(v1) tmp[3] = 1;
//
//   debug_write(tmp, 4);
//  }
    (span).primitive = (0x0009);
    (span).interpMask = (0);
    (span).arrayMask = (0x100);
    (span).start = 0;
    (span).end = (0);
    (span).facing = 0;
    (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays;

   /* determine bottom to top order of vertices */
   {
      GLfloat y0;
      GLfloat y1;
      GLfloat y2;
      y0 = v0->win[1];
      y1 = v1->win[1];
      y2 = v2->win[1];
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

   majDx = vMax->win[0] - vMin->win[0];
   majDy = vMax->win[1] - vMin->win[1];

   {
/* static */      const GLfloat botDx = vMid->win[0] - vMin->win[0];
/* static */      const GLfloat botDy = vMid->win[1] - vMin->win[1];
/* static */      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area == 0 || IS_INF_OR_NAN(area))
        return;
   }

#ifndef DO_OCCLUSION_TEST
   ctx->OcclusionResult = GL_TRUE;
#endif

   /* Plane equation setup:
    * We evaluate plane equations at window (x,y) coordinates in order
    * to compute color, Z, fog, texcoords, etc.  This isn't terribly
    * efficient but it's easy and reliable.
    */
   compute_plane(p0, p1, p2, p0[2], p1[2], p2[2], zPlane);
   span.arrayMask |= 0x008;
   compute_plane(p0, p1, p2, v0->fog, v1->fog, v2->fog, fogPlane);
   span.arrayMask |= 0x010;
   if (ctx->Light.ShadeModel == 0x1D01) {
      compute_plane(p0, p1, p2, v0->color[0], v1->color[0], v2->color[0], rPlane);
      compute_plane(p0, p1, p2, v0->color[1], v1->color[1], v2->color[1], gPlane);
      compute_plane(p0, p1, p2, v0->color[2], v1->color[2], v2->color[2], bPlane);
      compute_plane(p0, p1, p2, v0->color[3], v1->color[3], v2->color[3], aPlane);
   }
   else {
      constant_plane(v2->color[0], rPlane);
      constant_plane(v2->color[1], gPlane);
      constant_plane(v2->color[2], bPlane);
      constant_plane(v2->color[3], aPlane);
   }
   span.arrayMask |= 0x001;

   /* Begin bottom-to-top scan over the triangle.
    * The long edge will either be on the left or right side of the
    * triangle.  We always scan from the long edge toward the shorter
    * edges, stopping when we find that coverage = 0.  If the long edge
    * is on the left we scan left-to-right.  Else, we scan right-to-left.
    */
   yMin = vMin->win[1];
   yMax = vMax->win[1];
   iyMin = (GLint) yMin;
   iyMax = (GLint) yMax + 1;

/* EK */
   xMax = xMin = vMax->win[0];
   if(vMin->win[0] > xMax) xMax = vMin->win[0];
   if(vMin->win[0] < xMin) xMin = vMin->win[0];
   if(vMid->win[0] > xMax) xMax = vMid->win[0];
   if(vMid->win[0] < xMin) xMin = vMid->win[0];
   ixMin = (int) xMin;
   ixMax = (int) xMax + 1;
//   if(ixMin < 0) ixMin = 0;
   if(ixMax >= ctx->DrawBuffer->Width) ixMax = ctx->DrawBuffer->Width - 1;
   if(iyMax >= ctx->DrawBuffer->Height) iyMax = ctx->DrawBuffer->Height - 1;

   if (ltor) {
      /* scan left to right */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy < 0.0F ? -dxdy : 0.0F;

     x = pMin[0] - (yMin - iyMin) * dxdy;

      for (iy = iyMin; iy < iyMax; iy++, x += dxdy)
      {
         GLint ix, startX = (GLint) (x - xAdj);
         coverage = 0.0F;

         /* skip over fragments with zero coverage */
         while (startX < ixMax /* MAX_WIDTH EK */ ) { 
            coverage = compute_coveragef(pMin, pMid, pMax, startX, iy);
            if (coverage > 0.0F)
               break;
            startX++;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            /* (cx,cy) = center of fragment */
/* static */             const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
/* static */            struct span_arrays *array = span.array;
            array->coverage[count] = coverage;
            array->z[count] = (GLdepth) solve_plane(cx, cy, zPlane);
           array->fog[count] = solve_plane(cx, cy, fogPlane);
            array->rgba[count][0] = solve_plane_chan(cx, cy, rPlane);
            array->rgba[count][1] = solve_plane_chan(cx, cy, gPlane);
            array->rgba[count][2] = solve_plane_chan(cx, cy, bPlane);
            array->rgba[count][3] = solve_plane_chan(cx, cy, aPlane);
            ix++;
            if(ix >= ixMax)
                   break;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         if (ix <= startX)
            continue;

         span.x = startX;
         span.y = iy;
         span.end = (GLuint) ix - (GLuint) startX;
         _mesa_write_rgba_span(ctx, &span);
      }
   }
   else {
      /* scan right to left */
/* static */      const GLfloat *pMin = vMin->win;
/* static */      const GLfloat *pMid = vMid->win;
/* static */      const GLfloat *pMax = vMax->win;
/* static */      const GLfloat dxdy = majDx / majDy;
/* static */      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      x = pMin[0] - (yMin - iyMin) * dxdy;


      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
       coverage = 0.0F;
       startX = (GLint) (x + xAdj);

         /* make sure we're not past the window edge */
         if (startX >= ctx->DrawBuffer->_Xmax) {
            startX = ctx->DrawBuffer->_Xmax - 1;
         }

         /* skip fragments with zero coverage */
         while (startX  >= ixMin /* 0 EK */) {
            coverage = compute_coveragef(pMin, pMax, pMid, startX, iy);
            if (coverage > 0.0F)
               break;
            startX--;
         }

         /* enter interior of triangle */
         ix = startX;
         count = 0;
         while (coverage > 0.0F) {
            /* (cx,cy) = center of fragment */
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[ix] = coverage;
            array->z[ix] = (GLdepth) solve_plane(cx, cy, zPlane);
            array->fog[ix] = solve_plane(cx, cy, fogPlane);
            array->rgba[ix][0] = solve_plane_chan(cx, cy, rPlane);
            array->rgba[ix][1] = solve_plane_chan(cx, cy, gPlane);
            array->rgba[ix][2] = solve_plane_chan(cx, cy, bPlane);
            array->rgba[ix][3] = solve_plane_chan(cx, cy, aPlane);
            ix--;
            if(ix < ixMin)
                   break;
            count++;
            coverage = compute_coveragef(pMin, pMax, pMid, ix, iy);
         }

         if (startX <= ix)
            continue;

         n = (GLuint) startX - (GLuint) ix;

         left = ix + 1;

         /* shift all values to the left */
         /* XXX this is temporary */
         {
             struct span_arrays *array = span.array;
            for (j = 0; j < (GLint) n; j++)
            { jj = j + left;
               (array->rgba[j])[0] = (array->rgba[jj])[0];
               (array->rgba[j])[1] = (array->rgba[jj])[1];
               (array->rgba[j])[2] = (array->rgba[jj])[2];
               (array->rgba[j])[3] = (array->rgba[jj])[3];
               array->z[j]         = array->z[jj];
               array->fog[j]       = array->fog[jj];
               array->coverage[j]  = array->coverage[jj];
            }
         }

         span.x = left;
         span.y = iy;
         span.end = n;
         _mesa_write_rgba_span(ctx, &span);
      }
   }

// {
//      _mesa_debug(ctx, "rgba_aa_tri_End  : v0=%p,v1=%p, v2=%p\n",v0,v1,v2);
//      _mesa_debug(ctx, "(%f,%f,%f),(%f,%f,%f),(%f,%f,%f)\n",
//        v0->win[0],v0->win[1],v0->win[2],
//        v1->win[0],v1->win[1],v1->win[2],
//        v2->win[0],v2->win[1],v2->win[2] );
//
// }

}


