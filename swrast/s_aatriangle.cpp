/* $Id: s_aatriangle.c,v 1.26 2002/10/24 23:57:24 brianp Exp $ */
#include "glheader.h"
#include "macros.h"
#include "imports.h"
#include "mmath.h"
#include "s_aatriangle.h"
#include "s_context.h"
#include "s_span.h"

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
 * Return 1 / solve_plane().
 */
static  GLfloat  INLINE
solve_plane_recip(GLfloat x, GLfloat y, const GLfloat plane[4])
{
   const GLfloat denom = plane[3] + plane[0] * x + plane[1] * y;
   if (denom == 0.0F)
      return 0.0F;
   else
      return -plane[2] / denom;
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


#if 0
/*
 * Compute how much (area) of the given pixel is inside the triangle.
 * Vertices MUST be specified in counter-clockwise order.
 * Return:  coverage in [0, 1].
 */
/* static */ GLfloat
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
   const GLfloat dx2 = v0[0] - v2[0];
   const GLfloat dy2 = v0[1] - v2[1];
   GLint stop = 4, i;
   GLfloat insideCount = 16.0F;


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
#else
/* EVG code */

static  GLfloat
compute_coveragef(const GLfloat v0[3], const GLfloat v1[3],
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

#endif


/*
 * Compute how much (area) of the given pixel is inside the triangle.
 * Vertices MUST be specified in counter-clockwise order.
 * Return:  coverage in [0, 15].
 */
static GLint
compute_coveragei(const GLfloat v0[3], const GLfloat v1[3],
                  const GLfloat v2[3], GLint winx, GLint winy)
{
   /* NOTE: 15 samples instead of 16. */
   static const GLfloat samples[15][2] = {
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
      { (0.5+1*4+3)/16, (0.5+3*4+0)/16 }
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
/****************************/
#if 0

//EK DEBUG
int debug_write(int *buff, int n);


/* static */
void
rgba_aa_tri(GLcontext *ctx,
           const SWvertex *v0,
           const SWvertex *v1,
           const SWvertex *v2)
{

/*void triangle( GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint pv )*/
{
   const GLfloat *p0 = v0->win;
   const GLfloat *p1 = v1->win;
   const GLfloat *p2 = v2->win;
   const SWvertex *vMin, *vMid, *vMax;
   GLint iyMin, iyMax;
   GLfloat yMin, yMax;
   GLboolean ltor;
   GLfloat majDx, majDy;  /* major (i.e. long) edge dx and dy */

   struct sw_span span;

   GLfloat zPlane[4];
   GLfloat fogPlane[4];
   GLfloat rPlane[4], gPlane[4], bPlane[4], aPlane[4];
   GLfloat bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;

// {
//      _mesa_debug(ctx, "rgba_aa_tri_start: (%f,%f,%f),(%f,%f,%f),(%f,%f,%f)\n",
//        v0->win[0],v0->win[1],v0->win[2],
//        v1->win[0],v1->win[1],v1->win[2],
//        v2->win[0],v2->win[1],v2->win[2] );
//
// }
//printf(__FUNCTION__"ctx=%p,pv1=%p,pv2=%p,v3=%p",   ctx, v0,v1,v2);
//            printf(",v1=%i,v2=%i,3=%i\n",  *v0,*v1,*v2);
  { int tmp[4];
    tmp[0] = 0;
    if(ctx) tmp[0] = 1;
    tmp[1] = 0;
    if(v0) tmp[1] = 1;
    tmp[2] = 0;
    if(v1) tmp[2] = 1;
    tmp[3] = 0;
    if(v1) tmp[3] = 1;

   debug_write(tmp, 4);
  }
   do {
    (span).primitive = (0x0009);
    (span).interpMask = (0);
    (span).arrayMask = (0x100);
    (span).start = 0;
    (span).end = (0);
    (span).facing = 0;
    (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays;
   } while (0);

   /* determine bottom to top order of vertices */
   {
      GLfloat y0 = v0->win[1];
      GLfloat y1 = v1->win[1];
      GLfloat y2 = v2->win[1];
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
      const GLfloat botDx = vMid->win[0] - vMin->win[0];
      const GLfloat botDy = vMid->win[1] - vMin->win[1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area == 0 || IS_INF_OR_NAN(area))
        return;
   }

   ctx->OcclusionResult = 0x1;

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
      compute_plane(p0, p1, p2, v0->color[RCOMP], v1->color[RCOMP], v2->color[RCOMP], rPlane);
      compute_plane(p0, p1, p2, v0->color[GCOMP], v1->color[GCOMP], v2->color[GCOMP], gPlane);
      compute_plane(p0, p1, p2, v0->color[BCOMP], v1->color[BCOMP], v2->color[BCOMP], bPlane);
      compute_plane(p0, p1, p2, v0->color[ACOMP], v1->color[ACOMP], v2->color[ACOMP], aPlane);
   }
   else {
      constant_plane(v2->color[RCOMP], rPlane);
      constant_plane(v2->color[GCOMP], gPlane);
      constant_plane(v2->color[BCOMP], bPlane);
      constant_plane(v2->color[ACOMP], aPlane);
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

   if (ltor) {
      /* scan left to right */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      GLfloat x = pMin[0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count;
         GLfloat coverage = 0.0F;

         /* skip over fragments with zero coverage */
         while (startX < 2048) {
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
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[count] = coverage;
            array->z[count] = (GLdepth) solve_plane(cx, cy, zPlane);
           array->fog[count] = solve_plane(cx, cy, fogPlane);
            array->rgba[count][RCOMP] = solve_plane_chan(cx, cy, rPlane);
            array->rgba[count][GCOMP] = solve_plane_chan(cx, cy, gPlane);
            array->rgba[count][BCOMP] = solve_plane_chan(cx, cy, bPlane);
            array->rgba[count][ACOMP] = solve_plane_chan(cx, cy, aPlane);
            ix++;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         if (ix <= startX)
            continue;

         span.x = startX;
         span.y = iy;
         span.end = (GLuint) ix - (GLuint) startX;
         ;
         _mesa_write_rgba_span(ctx, &span);
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = pMin[0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage = 0.0F;

         /* make sure we're not past the window edge */
         if (startX >= ctx->DrawBuffer->_Xmax) {
            startX = ctx->DrawBuffer->_Xmax - 1;
         }

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
            /* (cx,cy) = center of fragment */
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[ix] = coverage;
            array->z[ix] = (GLdepth) solve_plane(cx, cy, zPlane);
            array->fog[ix] = solve_plane(cx, cy, fogPlane);
            array->rgba[ix][RCOMP] = solve_plane_chan(cx, cy, rPlane);
            array->rgba[ix][GCOMP] = solve_plane_chan(cx, cy, gPlane);
            array->rgba[ix][BCOMP] = solve_plane_chan(cx, cy, bPlane);
            array->rgba[ix][ACOMP] = solve_plane_chan(cx, cy, aPlane);
            ix--;
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
            GLint j;
            for (j = 0; j < (GLint) n; j++) {
               (array->rgba[j])[0] = (array->rgba[j + left])[0];
               (array->rgba[j])[1] = (array->rgba[j + left])[1];
               (array->rgba[j])[2] = (array->rgba[j + left])[2];
               (array->rgba[j])[3] = (array->rgba[j + left])[3];
               array->z[j] = array->z[j + left];
               array->fog[j] = array->fog[j + left];
               array->coverage[j] = array->coverage[j + left];
            }
         }

         span.x = left;
         span.y = iy;
         span.end = n;
         ;
         _mesa_write_rgba_span(ctx, &span);
      }
   }

// {
//      _mesa_debug(ctx, "rgba_aa_tri_End  : (%f,%f,%f),(%f,%f,%f),(%f,%f,%f)\n",
//        v0->win[0],v0->win[1],v0->win[2],
//        v1->win[0],v1->win[1],v1->win[2],
//        v2->win[0],v2->win[1],v2->win[2] );
//
// }

}


}
#else

F_swrast_tri_func rgba_aa_tri;

//void
//rgba_aa_tri(GLcontext *ctx,
//           const SWvertex *v0,
//           const SWvertex *v1,
//           const SWvertex *v2);

#endif
/************************************************/


static void
index_aa_tri(GLcontext *ctx,
            const SWvertex *v0,
            const SWvertex *v1,
            const SWvertex *v2)
{
{
   const GLfloat *p0 = v0->win;
   const GLfloat *p1 = v1->win;
   const GLfloat *p2 = v2->win;
   const SWvertex *vMin, *vMid, *vMax;
   GLint iyMin, iyMax;
   GLfloat yMin, yMax;
   GLboolean ltor;
   GLfloat majDx, majDy;  /* major (i.e. long) edge dx and dy */

   struct sw_span span;

   GLfloat zPlane[4];
   GLfloat fogPlane[4];
   GLfloat iPlane[4];
   GLfloat bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;


   do { (span).primitive = (0x0009); (span).interpMask = (0); (span).arrayMask = (0x100); (span).start = 0; (span).end = (0); (span).facing = 0; (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays; } while (0);

   /* determine bottom to top order of vertices */
   {
      GLfloat y0 = v0->win[1];
      GLfloat y1 = v1->win[1];
      GLfloat y2 = v2->win[1];
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
      const GLfloat botDx = vMid->win[0] - vMin->win[0];
      const GLfloat botDy = vMid->win[1] - vMin->win[1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area == 0 || IS_INF_OR_NAN(area))
        return;
   }

   ctx->OcclusionResult = 0x1;

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
      compute_plane(p0, p1, p2, (GLfloat) v0->index,
                    (GLfloat) v1->index, (GLfloat) v2->index, iPlane);
   }
   else {
      constant_plane((GLfloat) v2->index, iPlane);
   }
   span.arrayMask |= 0x004;

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

   if (ltor) {
      /* scan left to right */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      GLfloat x = pMin[0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count;
         GLfloat coverage = 0.0F;

         /* skip over fragments with zero coverage */
         while (startX < 2048) {
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
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[count] = (GLfloat) compute_coveragei(pMin, pMid, pMax, ix, iy);
            array->z[count] = (GLdepth) solve_plane(cx, cy, zPlane);
           array->fog[count] = solve_plane(cx, cy, fogPlane);
            array->index[count] = (GLint) solve_plane(cx, cy, iPlane);
            ix++;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         if (ix <= startX)
            continue;

         span.x = startX;
         span.y = iy;
         span.end = (GLuint) ix - (GLuint) startX;
         ;
         _mesa_write_index_span(ctx, &span);
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = pMin[0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage = 0.0F;

         /* make sure we're not past the window edge */
         if (startX >= ctx->DrawBuffer->_Xmax) {
            startX = ctx->DrawBuffer->_Xmax - 1;
         }

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
            /* (cx,cy) = center of fragment */
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[ix] = (GLfloat) compute_coveragei(pMin, pMax, pMid, ix, iy);
            array->z[ix] = (GLdepth) solve_plane(cx, cy, zPlane);
            array->fog[ix] = solve_plane(cx, cy, fogPlane);
            array->index[ix] = (GLint) solve_plane(cx, cy, iPlane);
            ix--;
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
            GLint j;
            for (j = 0; j < (GLint) n; j++) {
               array->index[j] = array->index[j + left];
               array->z[j] = array->z[j + left];
               array->fog[j] = array->fog[j + left];
               array->coverage[j] = array->coverage[j + left];
            }
         }

         span.x = left;
         span.y = iy;
         span.end = n;
         ;
         _mesa_write_index_span(ctx, &span);
      }
   }
}




}


/*
 * Compute mipmap level of detail.
 * XXX we should really include the R coordinate in this computation
 * in order to do 3-D texture mipmapping.
 */
static  GLfloat
compute_lambda(const GLfloat sPlane[4], const GLfloat tPlane[4],
               const GLfloat qPlane[4], GLfloat cx, GLfloat cy,
               GLfloat invQ, GLfloat texWidth, GLfloat texHeight)
{
   const GLfloat s = solve_plane(cx, cy, sPlane);
   const GLfloat t = solve_plane(cx, cy, tPlane);
   const GLfloat invQ_x1 = solve_plane_recip(cx+1.0F, cy, qPlane);
   const GLfloat invQ_y1 = solve_plane_recip(cx, cy+1.0F, qPlane);
   const GLfloat s_x1 = s - sPlane[0] / sPlane[2];
   const GLfloat s_y1 = s - sPlane[1] / sPlane[2];
   const GLfloat t_x1 = t - tPlane[0] / tPlane[2];
   const GLfloat t_y1 = t - tPlane[1] / tPlane[2];
   GLfloat dsdx = s_x1 * invQ_x1 - s * invQ;
   GLfloat dsdy = s_y1 * invQ_y1 - s * invQ;
   GLfloat dtdx = t_x1 * invQ_x1 - t * invQ;
   GLfloat dtdy = t_y1 * invQ_y1 - t * invQ;
   GLfloat maxU, maxV, rho, lambda;
   dsdx = ((GLfloat)__fabs( (dsdx) ));
   dsdy = ((GLfloat)__fabs( (dsdy) ));
   dtdx = ((GLfloat)__fabs( (dtdx) ));
   dtdy = ((GLfloat)__fabs( (dtdy) ));
   maxU = ( (dsdx)>(dsdy) ? (dsdx) : (dsdy) ) * texWidth;
   maxV = ( (dtdx)>(dtdy) ? (dtdx) : (dtdy) ) * texHeight;
   rho = ( (maxU)>(maxV) ? (maxU) : (maxV) );
   lambda = LOG2(rho);
   return lambda;
}


static void
tex_aa_tri(GLcontext *ctx,
          const SWvertex *v0,
          const SWvertex *v1,
          const SWvertex *v2)
{
   const GLfloat *p0 = v0->win;
   const GLfloat *p1 = v1->win;
   const GLfloat *p2 = v2->win;
   const SWvertex *vMin, *vMid, *vMax;
   GLint iyMin, iyMax;
   GLfloat yMin, yMax;
   GLboolean ltor;
   GLfloat majDx, majDy;  /* major (i.e. long) edge dx and dy */

   struct sw_span span;

   GLfloat zPlane[4];
   GLfloat fogPlane[4];
   GLfloat rPlane[4], gPlane[4], bPlane[4], aPlane[4];
   GLfloat sPlane[4], tPlane[4], uPlane[4], vPlane[4];
   GLfloat texWidth, texHeight;
   GLfloat bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;


   do { (span).primitive = (0x0009); (span).interpMask = (0); (span).arrayMask = (0x100); (span).start = 0; (span).end = (0); (span).facing = 0; (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays; } while (0);

   /* determine bottom to top order of vertices */
   {
      GLfloat y0 = v0->win[1];
      GLfloat y1 = v1->win[1];
      GLfloat y2 = v2->win[1];
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
      const GLfloat botDx = vMid->win[0] - vMin->win[0];
      const GLfloat botDy = vMid->win[1] - vMin->win[1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area == 0 || IS_INF_OR_NAN(area))
        return;
   }

   ctx->OcclusionResult = 0x1;

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
      compute_plane(p0, p1, p2, v0->color[RCOMP], v1->color[RCOMP], v2->color[RCOMP], rPlane);
      compute_plane(p0, p1, p2, v0->color[GCOMP], v1->color[GCOMP], v2->color[GCOMP], gPlane);
      compute_plane(p0, p1, p2, v0->color[BCOMP], v1->color[BCOMP], v2->color[BCOMP], bPlane);
      compute_plane(p0, p1, p2, v0->color[ACOMP], v1->color[ACOMP], v2->color[ACOMP], aPlane);
   }
   else {
      constant_plane(v2->color[RCOMP], rPlane);
      constant_plane(v2->color[GCOMP], gPlane);
      constant_plane(v2->color[BCOMP], bPlane);
      constant_plane(v2->color[ACOMP], aPlane);
   }
   span.arrayMask |= 0x001;
   {
      const struct gl_texture_object *obj = ctx->Texture.Unit[0]._Current;
      const struct gl_texture_image *texImage = obj->Image[obj->BaseLevel];
      const GLfloat invW0 = v0->win[3];
      const GLfloat invW1 = v1->win[3];
      const GLfloat invW2 = v2->win[3];
      const GLfloat s0 = v0->texcoord[0][0] * invW0;
      const GLfloat s1 = v1->texcoord[0][0] * invW1;
      const GLfloat s2 = v2->texcoord[0][0] * invW2;
      const GLfloat t0 = v0->texcoord[0][1] * invW0;
      const GLfloat t1 = v1->texcoord[0][1] * invW1;
      const GLfloat t2 = v2->texcoord[0][1] * invW2;
      const GLfloat r0 = v0->texcoord[0][2] * invW0;
      const GLfloat r1 = v1->texcoord[0][2] * invW1;
      const GLfloat r2 = v2->texcoord[0][2] * invW2;
      const GLfloat q0 = v0->texcoord[0][3] * invW0;
      const GLfloat q1 = v1->texcoord[0][3] * invW1;
      const GLfloat q2 = v2->texcoord[0][3] * invW2;
      compute_plane(p0, p1, p2, s0, s1, s2, sPlane);
      compute_plane(p0, p1, p2, t0, t1, t2, tPlane);
      compute_plane(p0, p1, p2, r0, r1, r2, uPlane);
      compute_plane(p0, p1, p2, q0, q1, q2, vPlane);
      texWidth = (GLfloat) texImage->Width;
      texHeight = (GLfloat) texImage->Height;
   }
   span.arrayMask |= (0x020 | 0x080);

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

   if (ltor) {
      /* scan left to right */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      GLfloat x = pMin[0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count;
         GLfloat coverage = 0.0F;

         /* skip over fragments with zero coverage */
         while (startX < 2048) {
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
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[count] = coverage;
            array->z[count] = (GLdepth) solve_plane(cx, cy, zPlane);
           array->fog[count] = solve_plane(cx, cy, fogPlane);
            array->rgba[count][RCOMP] = solve_plane_chan(cx, cy, rPlane);
            array->rgba[count][GCOMP] = solve_plane_chan(cx, cy, gPlane);
            array->rgba[count][BCOMP] = solve_plane_chan(cx, cy, bPlane);
            array->rgba[count][ACOMP] = solve_plane_chan(cx, cy, aPlane);
            {
               const GLfloat invQ = solve_plane_recip(cx, cy, vPlane);
               array->texcoords[0][count][0] = solve_plane(cx, cy, sPlane) * invQ;
               array->texcoords[0][count][1] = solve_plane(cx, cy, tPlane) * invQ;
               array->texcoords[0][count][2] = solve_plane(cx, cy, uPlane) * invQ;
               array->lambda[0][count] = compute_lambda(sPlane, tPlane, vPlane,
                                                      cx, cy, invQ,
                                                      texWidth, texHeight);
            }
            ix++;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         if (ix <= startX)
            continue;

         span.x = startX;
         span.y = iy;
         span.end = (GLuint) ix - (GLuint) startX;
         ;
         _mesa_write_texture_span(ctx, &span);
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = pMin[0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage = 0.0F;

         /* make sure we're not past the window edge */
         if (startX >= ctx->DrawBuffer->_Xmax) {
            startX = ctx->DrawBuffer->_Xmax - 1;
         }

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
            /* (cx,cy) = center of fragment */
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[ix] = coverage;
            array->z[ix] = (GLdepth) solve_plane(cx, cy, zPlane);
            array->fog[ix] = solve_plane(cx, cy, fogPlane);
            array->rgba[ix][RCOMP] = solve_plane_chan(cx, cy, rPlane);
            array->rgba[ix][GCOMP] = solve_plane_chan(cx, cy, gPlane);
            array->rgba[ix][BCOMP] = solve_plane_chan(cx, cy, bPlane);
            array->rgba[ix][ACOMP] = solve_plane_chan(cx, cy, aPlane);
            {
               const GLfloat invQ = solve_plane_recip(cx, cy, vPlane);
               array->texcoords[0][ix][0] = solve_plane(cx, cy, sPlane) * invQ;
               array->texcoords[0][ix][1] = solve_plane(cx, cy, tPlane) * invQ;
               array->texcoords[0][ix][2] = solve_plane(cx, cy, uPlane) * invQ;
               array->lambda[0][ix] = compute_lambda(sPlane, tPlane, vPlane,
                                          cx, cy, invQ, texWidth, texHeight);
            }
            ix--;
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
            GLint j;
            for (j = 0; j < (GLint) n; j++) {
                (array->rgba[j])[0] = (array->rgba[j + left])[0];
                (array->rgba[j])[1] = (array->rgba[j + left])[1];
                (array->rgba[j])[2] = (array->rgba[j + left])[2];
                (array->rgba[j])[3] = (array->rgba[j + left])[3];
               array->z[j] = array->z[j + left];
               array->fog[j] = array->fog[j + left];
               (array->texcoords[0][j])[0] = (array->texcoords[0][j + left])[0];
               (array->texcoords[0][j])[1] = (array->texcoords[0][j + left])[1];
               (array->texcoords[0][j])[2] = (array->texcoords[0][j + left])[2];
               (array->texcoords[0][j])[3] = (array->texcoords[0][j + left])[3];
               array->lambda[0][j] = array->lambda[0][j + left];
               array->coverage[j] = array->coverage[j + left];
            }
         }

         span.x = left;
         span.y = iy;
         span.end = n;
         ;
         _mesa_write_texture_span(ctx, &span);
      }
   }

}


static void
spec_tex_aa_tri(GLcontext *ctx,
               const SWvertex *v0,
               const SWvertex *v1,
               const SWvertex *v2)
{
   const GLfloat *p0 = v0->win;
   const GLfloat *p1 = v1->win;
   const GLfloat *p2 = v2->win;
   const SWvertex *vMin, *vMid, *vMax;
   GLint iyMin, iyMax;
   GLfloat yMin, yMax;
   GLboolean ltor;
   GLfloat majDx, majDy;  /* major (i.e. long) edge dx and dy */

   struct sw_span span;

   GLfloat zPlane[4];
   GLfloat fogPlane[4];
   GLfloat rPlane[4], gPlane[4], bPlane[4], aPlane[4];
   GLfloat srPlane[4], sgPlane[4], sbPlane[4];
   GLfloat sPlane[4], tPlane[4], uPlane[4], vPlane[4];
   GLfloat texWidth, texHeight;
   GLfloat bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;


   do { (span).primitive = (0x0009); (span).interpMask = (0); (span).arrayMask = (0x100); (span).start = 0; (span).end = (0); (span).facing = 0; (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays; } while (0);

   /* determine bottom to top order of vertices */
   {
      GLfloat y0 = v0->win[1];
      GLfloat y1 = v1->win[1];
      GLfloat y2 = v2->win[1];
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
      const GLfloat botDx = vMid->win[0] - vMin->win[0];
      const GLfloat botDy = vMid->win[1] - vMin->win[1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area == 0 || IS_INF_OR_NAN(area))
        return;
   }

   ctx->OcclusionResult = 0x1;

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
      compute_plane(p0, p1, p2, v0->color[RCOMP], v1->color[RCOMP], v2->color[RCOMP], rPlane);
      compute_plane(p0, p1, p2, v0->color[GCOMP], v1->color[GCOMP], v2->color[GCOMP], gPlane);
      compute_plane(p0, p1, p2, v0->color[BCOMP], v1->color[BCOMP], v2->color[BCOMP], bPlane);
      compute_plane(p0, p1, p2, v0->color[ACOMP], v1->color[ACOMP], v2->color[ACOMP], aPlane);
   }
   else {
      constant_plane(v2->color[RCOMP], rPlane);
      constant_plane(v2->color[GCOMP], gPlane);
      constant_plane(v2->color[BCOMP], bPlane);
      constant_plane(v2->color[ACOMP], aPlane);
   }
   span.arrayMask |= 0x001;
   if (ctx->Light.ShadeModel == 0x1D01) {
      compute_plane(p0, p1, p2, v0->specular[RCOMP], v1->specular[RCOMP], v2->specular[RCOMP],srPlane);
      compute_plane(p0, p1, p2, v0->specular[GCOMP], v1->specular[GCOMP], v2->specular[GCOMP],sgPlane);
      compute_plane(p0, p1, p2, v0->specular[BCOMP], v1->specular[BCOMP], v2->specular[BCOMP],sbPlane);
   }
   else {
      constant_plane(v2->specular[RCOMP], srPlane);
      constant_plane(v2->specular[GCOMP], sgPlane);
      constant_plane(v2->specular[BCOMP], sbPlane);
   }
   span.arrayMask |= 0x002;
   {
      const struct gl_texture_object *obj = ctx->Texture.Unit[0]._Current;
      const struct gl_texture_image *texImage = obj->Image[obj->BaseLevel];
      const GLfloat invW0 = v0->win[3];
      const GLfloat invW1 = v1->win[3];
      const GLfloat invW2 = v2->win[3];
      const GLfloat s0 = v0->texcoord[0][0] * invW0;
      const GLfloat s1 = v1->texcoord[0][0] * invW1;
      const GLfloat s2 = v2->texcoord[0][0] * invW2;
      const GLfloat t0 = v0->texcoord[0][1] * invW0;
      const GLfloat t1 = v1->texcoord[0][1] * invW1;
      const GLfloat t2 = v2->texcoord[0][1] * invW2;
      const GLfloat r0 = v0->texcoord[0][2] * invW0;
      const GLfloat r1 = v1->texcoord[0][2] * invW1;
      const GLfloat r2 = v2->texcoord[0][2] * invW2;
      const GLfloat q0 = v0->texcoord[0][3] * invW0;
      const GLfloat q1 = v1->texcoord[0][3] * invW1;
      const GLfloat q2 = v2->texcoord[0][3] * invW2;
      compute_plane(p0, p1, p2, s0, s1, s2, sPlane);
      compute_plane(p0, p1, p2, t0, t1, t2, tPlane);
      compute_plane(p0, p1, p2, r0, r1, r2, uPlane);
      compute_plane(p0, p1, p2, q0, q1, q2, vPlane);
      texWidth = (GLfloat) texImage->Width;
      texHeight = (GLfloat) texImage->Height;
   }
   span.arrayMask |= (0x020 | 0x080);

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

   if (ltor) {
      /* scan left to right */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      GLfloat x = pMin[0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count;
         GLfloat coverage = 0.0F;

         /* skip over fragments with zero coverage */
         while (startX < 2048) {
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
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[count] = coverage;
            array->z[count] = (GLdepth) solve_plane(cx, cy, zPlane);
           array->fog[count] = solve_plane(cx, cy, fogPlane);
            array->rgba[count][RCOMP] = solve_plane_chan(cx, cy, rPlane);
            array->rgba[count][GCOMP] = solve_plane_chan(cx, cy, gPlane);
            array->rgba[count][BCOMP] = solve_plane_chan(cx, cy, bPlane);
            array->rgba[count][ACOMP] = solve_plane_chan(cx, cy, aPlane);
            array->spec[count][RCOMP] = solve_plane_chan(cx, cy, srPlane);
            array->spec[count][GCOMP] = solve_plane_chan(cx, cy, sgPlane);
            array->spec[count][BCOMP] = solve_plane_chan(cx, cy, sbPlane);
            {
               const GLfloat invQ = solve_plane_recip(cx, cy, vPlane);
               array->texcoords[0][count][0] = solve_plane(cx, cy, sPlane) * invQ;
               array->texcoords[0][count][1] = solve_plane(cx, cy, tPlane) * invQ;
               array->texcoords[0][count][2] = solve_plane(cx, cy, uPlane) * invQ;
               array->lambda[0][count] = compute_lambda(sPlane, tPlane, vPlane,
                                                      cx, cy, invQ,
                                                      texWidth, texHeight);
            }
            ix++;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         if (ix <= startX)
            continue;

         span.x = startX;
         span.y = iy;
         span.end = (GLuint) ix - (GLuint) startX;
         _mesa_write_texture_span(ctx, &span);
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = pMin[0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage = 0.0F;

         /* make sure we're not past the window edge */
         if (startX >= ctx->DrawBuffer->_Xmax) {
            startX = ctx->DrawBuffer->_Xmax - 1;
         }

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
            /* (cx,cy) = center of fragment */
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[ix] = coverage;
            array->z[ix] = (GLdepth) solve_plane(cx, cy, zPlane);
            array->fog[ix] = solve_plane(cx, cy, fogPlane);
            array->rgba[ix][RCOMP] = solve_plane_chan(cx, cy, rPlane);
            array->rgba[ix][GCOMP] = solve_plane_chan(cx, cy, gPlane);
            array->rgba[ix][BCOMP] = solve_plane_chan(cx, cy, bPlane);
            array->rgba[ix][ACOMP] = solve_plane_chan(cx, cy, aPlane);
            array->spec[ix][RCOMP] = solve_plane_chan(cx, cy, srPlane);
            array->spec[ix][GCOMP] = solve_plane_chan(cx, cy, sgPlane);
            array->spec[ix][BCOMP] = solve_plane_chan(cx, cy, sbPlane);
            {
               const GLfloat invQ = solve_plane_recip(cx, cy, vPlane);
               array->texcoords[0][ix][0] = solve_plane(cx, cy, sPlane) * invQ;
               array->texcoords[0][ix][1] = solve_plane(cx, cy, tPlane) * invQ;
               array->texcoords[0][ix][2] = solve_plane(cx, cy, uPlane) * invQ;
               array->lambda[0][ix] = compute_lambda(sPlane, tPlane, vPlane,
                                          cx, cy, invQ, texWidth, texHeight);
            }
            ix--;
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
            GLint j;
            for (j = 0; j < (GLint) n; j++) {
                (array->rgba[j])[0] = (array->rgba[j + left])[0];
               (array->rgba[j])[1] = (array->rgba[j + left])[1];
               (array->rgba[j])[2] = (array->rgba[j + left])[2];
               (array->rgba[j])[3] = (array->rgba[j + left])[3];
                (array->spec[j])[0] = (array->spec[j + left])[0];
               (array->spec[j])[1] = (array->spec[j + left])[1];
               (array->spec[j])[2] = (array->spec[j + left])[2];
               (array->spec[j])[3] = (array->spec[j + left])[3];
               array->z[j] = array->z[j + left];
               array->fog[j] = array->fog[j + left];
               do { (array->texcoords[0][j])[0] = (array->texcoords[0][j + left])[0]; (array->texcoords[0][j])[1] = (array->texcoords[0][j + left])[1]; (array->texcoords[0][j])[2] = (array->texcoords[0][j + left])[2]; (array->texcoords[0][j])[3] = (array->texcoords[0][j + left])[3]; } while (0);
               array->lambda[0][j] = array->lambda[0][j + left];
               array->coverage[j] = array->coverage[j + left];
            }
         }

         span.x = left;
         span.y = iy;
         span.end = n;
         _mesa_write_texture_span(ctx, &span);
      }
   }
}


static void
multitex_aa_tri(GLcontext *ctx,
               const SWvertex *v0,
               const SWvertex *v1,
               const SWvertex *v2)
{
   const GLfloat *p0 = v0->win;
   const GLfloat *p1 = v1->win;
   const GLfloat *p2 = v2->win;
   const SWvertex *vMin, *vMid, *vMax;
   GLint iyMin, iyMax;
   GLfloat yMin, yMax;
   GLboolean ltor;
   GLfloat majDx, majDy;  /* major (i.e. long) edge dx and dy */

   struct sw_span span;

   GLfloat zPlane[4];
   GLfloat fogPlane[4];
   GLfloat rPlane[4], gPlane[4], bPlane[4], aPlane[4];
   GLfloat sPlane[8][4];  /* texture S */
   GLfloat tPlane[8][4];  /* texture T */
   GLfloat uPlane[8][4];  /* texture R */
   GLfloat vPlane[8][4];  /* texture Q */
   GLfloat texWidth[8], texHeight[8];
   GLfloat bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;


   do { (span).primitive = (0x0009); (span).interpMask = (0); (span).arrayMask = (0x100); (span).start = 0; (span).end = (0); (span).facing = 0; (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays; } while (0);

   /* determine bottom to top order of vertices */
   {
      GLfloat y0 = v0->win[1];
      GLfloat y1 = v1->win[1];
      GLfloat y2 = v2->win[1];
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
      const GLfloat botDx = vMid->win[0] - vMin->win[0];
      const GLfloat botDy = vMid->win[1] - vMin->win[1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area == 0 || IS_INF_OR_NAN(area))
        return;
   }

   ctx->OcclusionResult = 0x1;

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
      compute_plane(p0, p1, p2, v0->color[RCOMP], v1->color[RCOMP], v2->color[RCOMP], rPlane);
      compute_plane(p0, p1, p2, v0->color[GCOMP], v1->color[GCOMP], v2->color[GCOMP], gPlane);
      compute_plane(p0, p1, p2, v0->color[BCOMP], v1->color[BCOMP], v2->color[BCOMP], bPlane);
      compute_plane(p0, p1, p2, v0->color[ACOMP], v1->color[ACOMP], v2->color[ACOMP], aPlane);
   }
   else {
      constant_plane(v2->color[RCOMP], rPlane);
      constant_plane(v2->color[GCOMP], gPlane);
      constant_plane(v2->color[BCOMP], bPlane);
      constant_plane(v2->color[ACOMP], aPlane);
   }
   span.arrayMask |= 0x001;
   {
      GLuint u;
      for (u = 0; u < ctx->Const.MaxTextureUnits; u++) {
         if (ctx->Texture.Unit[u]._ReallyEnabled) {
            const struct gl_texture_object *obj = ctx->Texture.Unit[u]._Current;
            const struct gl_texture_image *texImage = obj->Image[obj->BaseLevel];
            const GLfloat invW0 = v0->win[3];
            const GLfloat invW1 = v1->win[3];
            const GLfloat invW2 = v2->win[3];
            const GLfloat s0 = v0->texcoord[u][0] * invW0;
            const GLfloat s1 = v1->texcoord[u][0] * invW1;
            const GLfloat s2 = v2->texcoord[u][0] * invW2;
            const GLfloat t0 = v0->texcoord[u][1] * invW0;
            const GLfloat t1 = v1->texcoord[u][1] * invW1;
            const GLfloat t2 = v2->texcoord[u][1] * invW2;
            const GLfloat r0 = v0->texcoord[u][2] * invW0;
            const GLfloat r1 = v1->texcoord[u][2] * invW1;
            const GLfloat r2 = v2->texcoord[u][2] * invW2;
            const GLfloat q0 = v0->texcoord[u][3] * invW0;
            const GLfloat q1 = v1->texcoord[u][3] * invW1;
            const GLfloat q2 = v2->texcoord[u][3] * invW2;
            compute_plane(p0, p1, p2, s0, s1, s2, sPlane[u]);
            compute_plane(p0, p1, p2, t0, t1, t2, tPlane[u]);
            compute_plane(p0, p1, p2, r0, r1, r2, uPlane[u]);
            compute_plane(p0, p1, p2, q0, q1, q2, vPlane[u]);
            texWidth[u]  = (GLfloat) texImage->Width;
            texHeight[u] = (GLfloat) texImage->Height;
         }
      }
   }
   span.arrayMask |= (0x020 | 0x080);

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

   if (ltor) {
      /* scan left to right */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      GLfloat x = pMin[0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count;
         GLfloat coverage = 0.0F;

         /* skip over fragments with zero coverage */
         while (startX < 2048) {
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
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[count] = coverage;
            array->z[count] = (GLdepth) solve_plane(cx, cy, zPlane);
           array->fog[count] = solve_plane(cx, cy, fogPlane);
            array->rgba[count][RCOMP] = solve_plane_chan(cx, cy, rPlane);
            array->rgba[count][GCOMP] = solve_plane_chan(cx, cy, gPlane);
            array->rgba[count][BCOMP] = solve_plane_chan(cx, cy, bPlane);
            array->rgba[count][ACOMP] = solve_plane_chan(cx, cy, aPlane);
            {
               GLuint unit;
               for (unit = 0; unit < ctx->Const.MaxTextureUnits; unit++) {
                  if (ctx->Texture.Unit[unit]._ReallyEnabled) {
                     GLfloat invQ = solve_plane_recip(cx, cy, vPlane[unit]);
                     array->texcoords[unit][count][0] = solve_plane(cx, cy, sPlane[unit]) * invQ;
                     array->texcoords[unit][count][1] = solve_plane(cx, cy, tPlane[unit]) * invQ;
                     array->texcoords[unit][count][2] = solve_plane(cx, cy, uPlane[unit]) * invQ;
                     array->lambda[unit][count] = compute_lambda(sPlane[unit],
                                      tPlane[unit], vPlane[unit], cx, cy, invQ,
                                      texWidth[unit], texHeight[unit]);
                  }
               }
            }
            ix++;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         if (ix <= startX)
            continue;

         span.x = startX;
         span.y = iy;
         span.end = (GLuint) ix - (GLuint) startX;
         ;
         _mesa_write_texture_span(ctx, &span);
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = pMin[0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage = 0.0F;

         /* make sure we're not past the window edge */
         if (startX >= ctx->DrawBuffer->_Xmax) {
            startX = ctx->DrawBuffer->_Xmax - 1;
         }

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
            /* (cx,cy) = center of fragment */
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[ix] = coverage;
            array->z[ix] = (GLdepth) solve_plane(cx, cy, zPlane);
            array->fog[ix] = solve_plane(cx, cy, fogPlane);
            array->rgba[ix][RCOMP] = solve_plane_chan(cx, cy, rPlane);
            array->rgba[ix][GCOMP] = solve_plane_chan(cx, cy, gPlane);
            array->rgba[ix][BCOMP] = solve_plane_chan(cx, cy, bPlane);
            array->rgba[ix][ACOMP] = solve_plane_chan(cx, cy, aPlane);
            {
               GLuint unit;
               for (unit = 0; unit < ctx->Const.MaxTextureUnits; unit++) {
                  if (ctx->Texture.Unit[unit]._ReallyEnabled) {
                     GLfloat invQ = solve_plane_recip(cx, cy, vPlane[unit]);
                     array->texcoords[unit][ix][0] = solve_plane(cx, cy, sPlane[unit]) * invQ;
                     array->texcoords[unit][ix][1] = solve_plane(cx, cy, tPlane[unit]) * invQ;
                     array->texcoords[unit][ix][2] = solve_plane(cx, cy, uPlane[unit]) * invQ;
                     array->lambda[unit][ix] = compute_lambda(sPlane[unit],
                                                            tPlane[unit],
                                                            vPlane[unit],
                                                            cx, cy, invQ,
                                                            texWidth[unit],
                                                            texHeight[unit]);
                  }
               }
            }
            ix--;
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
            GLint j;
            for (j = 0; j < (GLint) n; j++) {
               (array->rgba[j])[0] = (array->rgba[j + left])[0];
               (array->rgba[j])[1] = (array->rgba[j + left])[1];
               (array->rgba[j])[2] = (array->rgba[j + left])[2];
               (array->rgba[j])[3] = (array->rgba[j + left])[3];
               array->z[j] = array->z[j + left];
               array->fog[j] = array->fog[j + left];
               array->lambda[0][j] = array->lambda[0][j + left];
               array->coverage[j] = array->coverage[j + left];
            }
         }
         /* shift texcoords */
         {
            struct span_arrays *array = span.array;
            GLuint unit;
            for (unit = 0; unit < ctx->Const.MaxTextureUnits; unit++) {
               if (ctx->Texture.Unit[unit]._ReallyEnabled) {
                  GLint j;
                  for (j = 0; j < (GLint) n; j++) {
                    array->texcoords[unit][j][0] = array->texcoords[unit][j + left][0];
                     array->texcoords[unit][j][1] = array->texcoords[unit][j + left][1];
                     array->texcoords[unit][j][2] = array->texcoords[unit][j + left][2];
                     array->lambda[unit][j] = array->lambda[unit][j + left];
                  }
               }
            }
         }

         span.x = left;
         span.y = iy;
         span.end = n;
         _mesa_write_texture_span(ctx, &span);
      }
   }
}



static void
spec_multitex_aa_tri(GLcontext *ctx,
                    const SWvertex *v0,
                    const SWvertex *v1,
                    const SWvertex *v2)
{
   const GLfloat *p0 = v0->win;
   const GLfloat *p1 = v1->win;
   const GLfloat *p2 = v2->win;
   const SWvertex *vMin, *vMid, *vMax;
   GLint iyMin, iyMax;
   GLfloat yMin, yMax;
   GLboolean ltor;
   GLfloat majDx, majDy;  /* major (i.e. long) edge dx and dy */

   struct sw_span span;

   GLfloat zPlane[4];
   GLfloat fogPlane[4];
   GLfloat rPlane[4], gPlane[4], bPlane[4], aPlane[4];
   GLfloat srPlane[4], sgPlane[4], sbPlane[4];
   GLfloat sPlane[8][4];  /* texture S */
   GLfloat tPlane[8][4];  /* texture T */
   GLfloat uPlane[8][4];  /* texture R */
   GLfloat vPlane[8][4];  /* texture Q */
   GLfloat texWidth[8], texHeight[8];
   GLfloat bf = ((SWcontext *)ctx->swrast_context)->_backface_sign;


   do { (span).primitive = (0x0009); (span).interpMask = (0); (span).arrayMask = (0x100); (span).start = 0; (span).end = (0); (span).facing = 0; (span).array = ((SWcontext *)ctx->swrast_context)->SpanArrays; } while (0);

   /* determine bottom to top order of vertices */
   {
      GLfloat y0 = v0->win[1];
      GLfloat y1 = v1->win[1];
      GLfloat y2 = v2->win[1];
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
      const GLfloat botDx = vMid->win[0] - vMin->win[0];
      const GLfloat botDy = vMid->win[1] - vMin->win[1];
      const GLfloat area = majDx * botDy - botDx * majDy;
      ltor = (GLboolean) (area < 0.0F);
      /* Do backface culling */
      if (area * bf < 0 || area == 0 || IS_INF_OR_NAN(area))
        return;
   }

   ctx->OcclusionResult = 0x1;

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
      compute_plane(p0, p1, p2, v0->color[RCOMP], v1->color[RCOMP], v2->color[RCOMP], rPlane);
      compute_plane(p0, p1, p2, v0->color[GCOMP], v1->color[GCOMP], v2->color[GCOMP], gPlane);
      compute_plane(p0, p1, p2, v0->color[BCOMP], v1->color[BCOMP], v2->color[BCOMP], bPlane);
      compute_plane(p0, p1, p2, v0->color[ACOMP], v1->color[ACOMP], v2->color[ACOMP], aPlane);
   }
   else {
      constant_plane(v2->color[RCOMP], rPlane);
      constant_plane(v2->color[GCOMP], gPlane);
      constant_plane(v2->color[BCOMP], bPlane);
      constant_plane(v2->color[ACOMP], aPlane);
   }
   span.arrayMask |= 0x001;
   if (ctx->Light.ShadeModel == 0x1D01) {
      compute_plane(p0, p1, p2, v0->specular[RCOMP], v1->specular[RCOMP], v2->specular[RCOMP],srPlane);
      compute_plane(p0, p1, p2, v0->specular[GCOMP], v1->specular[GCOMP], v2->specular[GCOMP],sgPlane);
      compute_plane(p0, p1, p2, v0->specular[BCOMP], v1->specular[BCOMP], v2->specular[BCOMP],sbPlane);
   }
   else {
      constant_plane(v2->specular[RCOMP], srPlane);
      constant_plane(v2->specular[GCOMP], sgPlane);
      constant_plane(v2->specular[BCOMP], sbPlane);
   }
   span.arrayMask |= 0x002;
   {
      GLuint u;
      for (u = 0; u < ctx->Const.MaxTextureUnits; u++) {
         if (ctx->Texture.Unit[u]._ReallyEnabled) {
            const struct gl_texture_object *obj = ctx->Texture.Unit[u]._Current;
            const struct gl_texture_image *texImage = obj->Image[obj->BaseLevel];
            const GLfloat invW0 = v0->win[3];
            const GLfloat invW1 = v1->win[3];
            const GLfloat invW2 = v2->win[3];
            const GLfloat s0 = v0->texcoord[u][0] * invW0;
            const GLfloat s1 = v1->texcoord[u][0] * invW1;
            const GLfloat s2 = v2->texcoord[u][0] * invW2;
            const GLfloat t0 = v0->texcoord[u][1] * invW0;
            const GLfloat t1 = v1->texcoord[u][1] * invW1;
            const GLfloat t2 = v2->texcoord[u][1] * invW2;
            const GLfloat r0 = v0->texcoord[u][2] * invW0;
            const GLfloat r1 = v1->texcoord[u][2] * invW1;
            const GLfloat r2 = v2->texcoord[u][2] * invW2;
            const GLfloat q0 = v0->texcoord[u][3] * invW0;
            const GLfloat q1 = v1->texcoord[u][3] * invW1;
            const GLfloat q2 = v2->texcoord[u][3] * invW2;
            compute_plane(p0, p1, p2, s0, s1, s2, sPlane[u]);
            compute_plane(p0, p1, p2, t0, t1, t2, tPlane[u]);
            compute_plane(p0, p1, p2, r0, r1, r2, uPlane[u]);
            compute_plane(p0, p1, p2, q0, q1, q2, vPlane[u]);
            texWidth[u]  = (GLfloat) texImage->Width;
            texHeight[u] = (GLfloat) texImage->Height;
         }
      }
   }
   span.arrayMask |= (0x020 | 0x080);

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

   if (ltor) {
      /* scan left to right */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy < 0.0F ? -dxdy : 0.0F;
      GLfloat x = pMin[0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, startX = (GLint) (x - xAdj);
         GLuint count;
         GLfloat coverage = 0.0F;

         /* skip over fragments with zero coverage */
         while (startX < 2048) {
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
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[count] = coverage;
            array->z[count] = (GLdepth) solve_plane(cx, cy, zPlane);
           array->fog[count] = solve_plane(cx, cy, fogPlane);
            array->rgba[count][RCOMP] = solve_plane_chan(cx, cy, rPlane);
            array->rgba[count][GCOMP] = solve_plane_chan(cx, cy, gPlane);
            array->rgba[count][BCOMP] = solve_plane_chan(cx, cy, bPlane);
            array->rgba[count][ACOMP] = solve_plane_chan(cx, cy, aPlane);
            array->spec[count][RCOMP] = solve_plane_chan(cx, cy, srPlane);
            array->spec[count][GCOMP] = solve_plane_chan(cx, cy, sgPlane);
            array->spec[count][BCOMP] = solve_plane_chan(cx, cy, sbPlane);
            {
               GLuint unit;
               for (unit = 0; unit < ctx->Const.MaxTextureUnits; unit++) {
                  if (ctx->Texture.Unit[unit]._ReallyEnabled) {
                     GLfloat invQ = solve_plane_recip(cx, cy, vPlane[unit]);
                     array->texcoords[unit][count][0] = solve_plane(cx, cy, sPlane[unit]) * invQ;
                     array->texcoords[unit][count][1] = solve_plane(cx, cy, tPlane[unit]) * invQ;
                     array->texcoords[unit][count][2] = solve_plane(cx, cy, uPlane[unit]) * invQ;
                     array->lambda[unit][count] = compute_lambda(sPlane[unit],
                                      tPlane[unit], vPlane[unit], cx, cy, invQ,
                                      texWidth[unit], texHeight[unit]);
                  }
               }
            }
            ix++;
            count++;
            coverage = compute_coveragef(pMin, pMid, pMax, ix, iy);
         }

         if (ix <= startX)
            continue;

         span.x = startX;
         span.y = iy;
         span.end = (GLuint) ix - (GLuint) startX;
         ;
         _mesa_write_texture_span(ctx, &span);
      }
   }
   else {
      /* scan right to left */
      const GLfloat *pMin = vMin->win;
      const GLfloat *pMid = vMid->win;
      const GLfloat *pMax = vMax->win;
      const GLfloat dxdy = majDx / majDy;
      const GLfloat xAdj = dxdy > 0 ? dxdy : 0.0F;
      GLfloat x = pMin[0] - (yMin - iyMin) * dxdy;
      GLint iy;
      for (iy = iyMin; iy < iyMax; iy++, x += dxdy) {
         GLint ix, left, startX = (GLint) (x + xAdj);
         GLuint count, n;
         GLfloat coverage = 0.0F;

         /* make sure we're not past the window edge */
         if (startX >= ctx->DrawBuffer->_Xmax) {
            startX = ctx->DrawBuffer->_Xmax - 1;
         }

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
            /* (cx,cy) = center of fragment */
            const GLfloat cx = ix + 0.5F, cy = iy + 0.5F;
            struct span_arrays *array = span.array;
            array->coverage[ix] = coverage;
            array->z[ix] = (GLdepth) solve_plane(cx, cy, zPlane);
            array->fog[ix] = solve_plane(cx, cy, fogPlane);
            array->rgba[ix][RCOMP] = solve_plane_chan(cx, cy, rPlane);
            array->rgba[ix][GCOMP] = solve_plane_chan(cx, cy, gPlane);
            array->rgba[ix][BCOMP] = solve_plane_chan(cx, cy, bPlane);
            array->rgba[ix][ACOMP] = solve_plane_chan(cx, cy, aPlane);
            array->spec[ix][RCOMP] = solve_plane_chan(cx, cy, srPlane);
            array->spec[ix][GCOMP] = solve_plane_chan(cx, cy, sgPlane);
            array->spec[ix][BCOMP] = solve_plane_chan(cx, cy, sbPlane);
            {
               GLuint unit;
               for (unit = 0; unit < ctx->Const.MaxTextureUnits; unit++) {
                  if (ctx->Texture.Unit[unit]._ReallyEnabled) {
                     GLfloat invQ = solve_plane_recip(cx, cy, vPlane[unit]);
                     array->texcoords[unit][ix][0] = solve_plane(cx, cy, sPlane[unit]) * invQ;
                     array->texcoords[unit][ix][1] = solve_plane(cx, cy, tPlane[unit]) * invQ;
                     array->texcoords[unit][ix][2] = solve_plane(cx, cy, uPlane[unit]) * invQ;
                     array->lambda[unit][ix] = compute_lambda(sPlane[unit],
                                                            tPlane[unit],
                                                            vPlane[unit],
                                                            cx, cy, invQ,
                                                            texWidth[unit],
                                                            texHeight[unit]);
                  }
               }
            }
            ix--;
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
            GLint j;
            for (j = 0; j < (GLint) n; j++) {
               (array->rgba[j])[0] = (array->rgba[j + left])[0];
               (array->rgba[j])[1] = (array->rgba[j + left])[1];
              (array->rgba[j])[2] = (array->rgba[j + left])[2];
              (array->rgba[j])[3] = (array->rgba[j + left])[3];
               (array->spec[j])[0] = (array->spec[j + left])[0];
              (array->spec[j])[1] = (array->spec[j + left])[1];
              (array->spec[j])[2] = (array->spec[j + left])[2];
              (array->spec[j])[3] = (array->spec[j + left])[3];
               array->z[j] = array->z[j + left];
               array->fog[j] = array->fog[j + left];
               array->lambda[0][j] = array->lambda[0][j + left];
               array->coverage[j] = array->coverage[j + left];
            }
         }
         /* shift texcoords */
         {
            struct span_arrays *array = span.array;
            GLuint unit;
            for (unit = 0; unit < ctx->Const.MaxTextureUnits; unit++) {
               if (ctx->Texture.Unit[unit]._ReallyEnabled) {
                  GLint j;
                  for (j = 0; j < (GLint) n; j++) {
                    array->texcoords[unit][j][0] = array->texcoords[unit][j + left][0];
                     array->texcoords[unit][j][1] = array->texcoords[unit][j + left][1];
                     array->texcoords[unit][j][2] = array->texcoords[unit][j + left][2];
                     array->lambda[unit][j] = array->lambda[unit][j + left];
                  }
               }
            }
         }

         span.x = left;
         span.y = iy;
         span.end = n;
         _mesa_write_texture_span(ctx, &span);
      }
   }
}


/*
 * Examine GL state and set swrast->Triangle to an
 * appropriate antialiased triangle rasterizer function.
 */
void
_mesa_set_aa_triangle_function(GLcontext *ctx)
{

   if (ctx->Texture._EnabledUnits != 0) {
      if (ctx->_TriangleCaps & 0x2) {
         if (ctx->Texture._EnabledUnits > 1) {
            ((SWcontext *)ctx->swrast_context)->Triangle = spec_multitex_aa_tri;
         }
         else {
            ((SWcontext *)ctx->swrast_context)->Triangle = spec_tex_aa_tri;
         }
      }
      else {
         if (ctx->Texture._EnabledUnits > 1) {
            ((SWcontext *)ctx->swrast_context)->Triangle = multitex_aa_tri;
         }
         else {
            ((SWcontext *)ctx->swrast_context)->Triangle = tex_aa_tri;
         }
      }
   }
   else if (ctx->Visual.rgbMode) {
      ((SWcontext *)ctx->swrast_context)->Triangle = rgba_aa_tri;
   }
   else {
      ((SWcontext *)ctx->swrast_context)->Triangle = index_aa_tri;
   }

   ;
}
