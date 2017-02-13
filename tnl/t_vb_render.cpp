/* $Id: t_vb_render.c,v 1.33 2002/10/29 20:29:04 brianp Exp $ */
/*
 * Render whole vertex buffers, including projection of vertices from
 * clip space and clipping of primitives.
 *
 * This file makes calls to project vertices and to the point, line
 * and triangle rasterizers via the function pointers:
 *
 *    context->Driver.Render.*
 *
 */


#include "glheader.h"
#include "context.h"
#include "enums.h"
#include "macros.h"
#include "imports.h"
#include "mtypes.h"
#include "mmath.h"

#include "math/m_matrix.h"
#include "math/m_xform.h"

#include "t_pipeline.h"



/**********************************************************************/
/*                        Clip single primitives                      */
/**********************************************************************/




/* $Id: t_vb_cliptmp.h,v 1.16 2002/10/29 20:29:03 brianp Exp $ */





/* Clip a line against the viewport and user clip planes.
 */
static  void
clip_line_4( GLcontext *ctx, GLuint i, GLuint j, GLubyte mask )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   interp_func interp = tnl->Driver.Render.Interp;
   GLfloat (*coord)[4] = VB->ClipPtr->data;
   GLuint ii = i, jj = j, p;

   VB->LastClipped = VB->FirstClipped;

   if (mask & 0x3f) {
      do { if (mask & 0x01) { GLfloat dpI = coord[ii][0]*-1 + coord[ii][1]*0 + coord[ii][2]*0 + coord[ii][3]*1; GLfloat dpJ = coord[jj][0]*-1 + coord[jj][1]*0 + coord[jj][2]*0 + coord[jj][3]*1; if (((((fi_type *) &(dpI))->i ^ ((fi_type *) &(dpJ))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; if ((((fi_type *) &(dpJ))->i & (1<<31))) { GLfloat t = dpI / (dpI - dpJ); VB->ClipMask[jj] |= 0x01; do { coord[newvert][0] = (((coord[ii])[0]) + ((t)) * (((coord[jj])[0]) - ((coord[ii])[0]))); coord[newvert][1] = (((coord[ii])[1]) + ((t)) * (((coord[jj])[1]) - ((coord[ii])[1]))); coord[newvert][2] = (((coord[ii])[2]) + ((t)) * (((coord[jj])[2]) - ((coord[ii])[2]))); coord[newvert][3] = (((coord[ii])[3]) + ((t)) * (((coord[jj])[3]) - ((coord[ii])[3]))); } while (0); interp( ctx, t, newvert, ii, jj, 0x0 ); jj = newvert; } else { GLfloat t = dpJ / (dpJ - dpI); VB->ClipMask[ii] |= 0x01; do { coord[newvert][0] = (((coord[jj])[0]) + ((t)) * (((coord[ii])[0]) - ((coord[jj])[0]))); coord[newvert][1] = (((coord[jj])[1]) + ((t)) * (((coord[ii])[1]) - ((coord[jj])[1]))); coord[newvert][2] = (((coord[jj])[2]) + ((t)) * (((coord[ii])[2]) - ((coord[jj])[2]))); coord[newvert][3] = (((coord[jj])[3]) + ((t)) * (((coord[ii])[3]) - ((coord[jj])[3]))); } while (0); interp( ctx, t, newvert, jj, ii, 0x0 ); ii = newvert; } } else if ((((fi_type *) &(dpI))->i & (1<<31))) return; } } while (0);
      do { if (mask & 0x02) { GLfloat dpI = coord[ii][0]*1 + coord[ii][1]*0 + coord[ii][2]*0 + coord[ii][3]*1; GLfloat dpJ = coord[jj][0]*1 + coord[jj][1]*0 + coord[jj][2]*0 + coord[jj][3]*1; if (((((fi_type *) &(dpI))->i ^ ((fi_type *) &(dpJ))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; if ((((fi_type *) &(dpJ))->i & (1<<31))) { GLfloat t = dpI / (dpI - dpJ); VB->ClipMask[jj] |= 0x02; do { coord[newvert][0] = (((coord[ii])[0]) + ((t)) * (((coord[jj])[0]) - ((coord[ii])[0]))); coord[newvert][1] = (((coord[ii])[1]) + ((t)) * (((coord[jj])[1]) - ((coord[ii])[1]))); coord[newvert][2] = (((coord[ii])[2]) + ((t)) * (((coord[jj])[2]) - ((coord[ii])[2]))); coord[newvert][3] = (((coord[ii])[3]) + ((t)) * (((coord[jj])[3]) - ((coord[ii])[3]))); } while (0); interp( ctx, t, newvert, ii, jj, 0x0 ); jj = newvert; } else { GLfloat t = dpJ / (dpJ - dpI); VB->ClipMask[ii] |= 0x02; do { coord[newvert][0] = (((coord[jj])[0]) + ((t)) * (((coord[ii])[0]) - ((coord[jj])[0]))); coord[newvert][1] = (((coord[jj])[1]) + ((t)) * (((coord[ii])[1]) - ((coord[jj])[1]))); coord[newvert][2] = (((coord[jj])[2]) + ((t)) * (((coord[ii])[2]) - ((coord[jj])[2]))); coord[newvert][3] = (((coord[jj])[3]) + ((t)) * (((coord[ii])[3]) - ((coord[jj])[3]))); } while (0); interp( ctx, t, newvert, jj, ii, 0x0 ); ii = newvert; } } else if ((((fi_type *) &(dpI))->i & (1<<31))) return; } } while (0);
      do { if (mask & 0x04) { GLfloat dpI = coord[ii][0]*0 + coord[ii][1]*-1 + coord[ii][2]*0 + coord[ii][3]*1; GLfloat dpJ = coord[jj][0]*0 + coord[jj][1]*-1 + coord[jj][2]*0 + coord[jj][3]*1; if (((((fi_type *) &(dpI))->i ^ ((fi_type *) &(dpJ))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; if ((((fi_type *) &(dpJ))->i & (1<<31))) { GLfloat t = dpI / (dpI - dpJ); VB->ClipMask[jj] |= 0x04; do { coord[newvert][0] = (((coord[ii])[0]) + ((t)) * (((coord[jj])[0]) - ((coord[ii])[0]))); coord[newvert][1] = (((coord[ii])[1]) + ((t)) * (((coord[jj])[1]) - ((coord[ii])[1]))); coord[newvert][2] = (((coord[ii])[2]) + ((t)) * (((coord[jj])[2]) - ((coord[ii])[2]))); coord[newvert][3] = (((coord[ii])[3]) + ((t)) * (((coord[jj])[3]) - ((coord[ii])[3]))); } while (0); interp( ctx, t, newvert, ii, jj, 0x0 ); jj = newvert; } else { GLfloat t = dpJ / (dpJ - dpI); VB->ClipMask[ii] |= 0x04; do { coord[newvert][0] = (((coord[jj])[0]) + ((t)) * (((coord[ii])[0]) - ((coord[jj])[0]))); coord[newvert][1] = (((coord[jj])[1]) + ((t)) * (((coord[ii])[1]) - ((coord[jj])[1]))); coord[newvert][2] = (((coord[jj])[2]) + ((t)) * (((coord[ii])[2]) - ((coord[jj])[2]))); coord[newvert][3] = (((coord[jj])[3]) + ((t)) * (((coord[ii])[3]) - ((coord[jj])[3]))); } while (0); interp( ctx, t, newvert, jj, ii, 0x0 ); ii = newvert; } } else if ((((fi_type *) &(dpI))->i & (1<<31))) return; } } while (0);
      do { if (mask & 0x08) { GLfloat dpI = coord[ii][0]*0 + coord[ii][1]*1 + coord[ii][2]*0 + coord[ii][3]*1; GLfloat dpJ = coord[jj][0]*0 + coord[jj][1]*1 + coord[jj][2]*0 + coord[jj][3]*1; if (((((fi_type *) &(dpI))->i ^ ((fi_type *) &(dpJ))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; if ((((fi_type *) &(dpJ))->i & (1<<31))) { GLfloat t = dpI / (dpI - dpJ); VB->ClipMask[jj] |= 0x08; do { coord[newvert][0] = (((coord[ii])[0]) + ((t)) * (((coord[jj])[0]) - ((coord[ii])[0]))); coord[newvert][1] = (((coord[ii])[1]) + ((t)) * (((coord[jj])[1]) - ((coord[ii])[1]))); coord[newvert][2] = (((coord[ii])[2]) + ((t)) * (((coord[jj])[2]) - ((coord[ii])[2]))); coord[newvert][3] = (((coord[ii])[3]) + ((t)) * (((coord[jj])[3]) - ((coord[ii])[3]))); } while (0); interp( ctx, t, newvert, ii, jj, 0x0 ); jj = newvert; } else { GLfloat t = dpJ / (dpJ - dpI); VB->ClipMask[ii] |= 0x08; do { coord[newvert][0] = (((coord[jj])[0]) + ((t)) * (((coord[ii])[0]) - ((coord[jj])[0]))); coord[newvert][1] = (((coord[jj])[1]) + ((t)) * (((coord[ii])[1]) - ((coord[jj])[1]))); coord[newvert][2] = (((coord[jj])[2]) + ((t)) * (((coord[ii])[2]) - ((coord[jj])[2]))); coord[newvert][3] = (((coord[jj])[3]) + ((t)) * (((coord[ii])[3]) - ((coord[jj])[3]))); } while (0); interp( ctx, t, newvert, jj, ii, 0x0 ); ii = newvert; } } else if ((((fi_type *) &(dpI))->i & (1<<31))) return; } } while (0);
      do { if (mask & 0x20) { GLfloat dpI = coord[ii][0]*0 + coord[ii][1]*0 + coord[ii][2]*-1 + coord[ii][3]*1; GLfloat dpJ = coord[jj][0]*0 + coord[jj][1]*0 + coord[jj][2]*-1 + coord[jj][3]*1; if (((((fi_type *) &(dpI))->i ^ ((fi_type *) &(dpJ))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; if ((((fi_type *) &(dpJ))->i & (1<<31))) { GLfloat t = dpI / (dpI - dpJ); VB->ClipMask[jj] |= 0x20; do { coord[newvert][0] = (((coord[ii])[0]) + ((t)) * (((coord[jj])[0]) - ((coord[ii])[0]))); coord[newvert][1] = (((coord[ii])[1]) + ((t)) * (((coord[jj])[1]) - ((coord[ii])[1]))); coord[newvert][2] = (((coord[ii])[2]) + ((t)) * (((coord[jj])[2]) - ((coord[ii])[2]))); coord[newvert][3] = (((coord[ii])[3]) + ((t)) * (((coord[jj])[3]) - ((coord[ii])[3]))); } while (0); interp( ctx, t, newvert, ii, jj, 0x0 ); jj = newvert; } else { GLfloat t = dpJ / (dpJ - dpI); VB->ClipMask[ii] |= 0x20; do { coord[newvert][0] = (((coord[jj])[0]) + ((t)) * (((coord[ii])[0]) - ((coord[jj])[0]))); coord[newvert][1] = (((coord[jj])[1]) + ((t)) * (((coord[ii])[1]) - ((coord[jj])[1]))); coord[newvert][2] = (((coord[jj])[2]) + ((t)) * (((coord[ii])[2]) - ((coord[jj])[2]))); coord[newvert][3] = (((coord[jj])[3]) + ((t)) * (((coord[ii])[3]) - ((coord[jj])[3]))); } while (0); interp( ctx, t, newvert, jj, ii, 0x0 ); ii = newvert; } } else if ((((fi_type *) &(dpI))->i & (1<<31))) return; } } while (0);
      do { if (mask & 0x10) { GLfloat dpI = coord[ii][0]*0 + coord[ii][1]*0 + coord[ii][2]*1 + coord[ii][3]*1; GLfloat dpJ = coord[jj][0]*0 + coord[jj][1]*0 + coord[jj][2]*1 + coord[jj][3]*1; if (((((fi_type *) &(dpI))->i ^ ((fi_type *) &(dpJ))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; if ((((fi_type *) &(dpJ))->i & (1<<31))) { GLfloat t = dpI / (dpI - dpJ); VB->ClipMask[jj] |= 0x10; do { coord[newvert][0] = (((coord[ii])[0]) + ((t)) * (((coord[jj])[0]) - ((coord[ii])[0]))); coord[newvert][1] = (((coord[ii])[1]) + ((t)) * (((coord[jj])[1]) - ((coord[ii])[1]))); coord[newvert][2] = (((coord[ii])[2]) + ((t)) * (((coord[jj])[2]) - ((coord[ii])[2]))); coord[newvert][3] = (((coord[ii])[3]) + ((t)) * (((coord[jj])[3]) - ((coord[ii])[3]))); } while (0); interp( ctx, t, newvert, ii, jj, 0x0 ); jj = newvert; } else { GLfloat t = dpJ / (dpJ - dpI); VB->ClipMask[ii] |= 0x10; do { coord[newvert][0] = (((coord[jj])[0]) + ((t)) * (((coord[ii])[0]) - ((coord[jj])[0]))); coord[newvert][1] = (((coord[jj])[1]) + ((t)) * (((coord[ii])[1]) - ((coord[jj])[1]))); coord[newvert][2] = (((coord[jj])[2]) + ((t)) * (((coord[ii])[2]) - ((coord[jj])[2]))); coord[newvert][3] = (((coord[jj])[3]) + ((t)) * (((coord[ii])[3]) - ((coord[jj])[3]))); } while (0); interp( ctx, t, newvert, jj, ii, 0x0 ); ii = newvert; } } else if ((((fi_type *) &(dpI))->i & (1<<31))) return; } } while (0);
   }

   if (mask & 0x40) {
      for (p=0;p<6;p++) {
	 if (ctx->Transform.ClipPlanesEnabled & (1 << p)) {
            const GLfloat a = ctx->Transform._ClipUserPlane[p][0];
            const GLfloat b = ctx->Transform._ClipUserPlane[p][1];
            const GLfloat c = ctx->Transform._ClipUserPlane[p][2];
            const GLfloat d = ctx->Transform._ClipUserPlane[p][3];
	    do { if (mask & 0x40) { GLfloat dpI = coord[ii][0]*a + coord[ii][1]*b + coord[ii][2]*c + coord[ii][3]*d; GLfloat dpJ = coord[jj][0]*a + coord[jj][1]*b + coord[jj][2]*c + coord[jj][3]*d; if (((((fi_type *) &(dpI))->i ^ ((fi_type *) &(dpJ))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; if ((((fi_type *) &(dpJ))->i & (1<<31))) { GLfloat t = dpI / (dpI - dpJ); VB->ClipMask[jj] |= 0x40; do { coord[newvert][0] = (((coord[ii])[0]) + ((t)) * (((coord[jj])[0]) - ((coord[ii])[0]))); coord[newvert][1] = (((coord[ii])[1]) + ((t)) * (((coord[jj])[1]) - ((coord[ii])[1]))); coord[newvert][2] = (((coord[ii])[2]) + ((t)) * (((coord[jj])[2]) - ((coord[ii])[2]))); coord[newvert][3] = (((coord[ii])[3]) + ((t)) * (((coord[jj])[3]) - ((coord[ii])[3]))); } while (0); interp( ctx, t, newvert, ii, jj, 0x0 ); jj = newvert; } else { GLfloat t = dpJ / (dpJ - dpI); VB->ClipMask[ii] |= 0x40; do { coord[newvert][0] = (((coord[jj])[0]) + ((t)) * (((coord[ii])[0]) - ((coord[jj])[0]))); coord[newvert][1] = (((coord[jj])[1]) + ((t)) * (((coord[ii])[1]) - ((coord[jj])[1]))); coord[newvert][2] = (((coord[jj])[2]) + ((t)) * (((coord[ii])[2]) - ((coord[jj])[2]))); coord[newvert][3] = (((coord[jj])[3]) + ((t)) * (((coord[ii])[3]) - ((coord[jj])[3]))); } while (0); interp( ctx, t, newvert, jj, ii, 0x0 ); ii = newvert; } } else if ((((fi_type *) &(dpI))->i & (1<<31))) return; } } while (0);
	 }
      }
   }

   if ((ctx->_TriangleCaps & 0x1) && j != jj)
      tnl->Driver.Render.CopyPV( ctx, jj, j );

   tnl->Driver.Render.ClippedLine( ctx, ii, jj );
}


/* Clip a triangle against the viewport and user clip planes.
 */
static  void
clip_tri_4( GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLubyte mask )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   interp_func interp = tnl->Driver.Render.Interp;
   GLfloat (*coord)[4] = VB->ClipPtr->data;
   GLuint pv = v2;
   GLuint vlist[2][((2 * (6 + 6))+1)];
   GLuint *inlist = vlist[0], *outlist = vlist[1];
   GLuint p;
   GLubyte *clipmask = VB->ClipMask;
   GLuint n = 3;

   do { inlist[0] = v2; inlist[1] = v0; inlist[2] = v1; } while(0); /* pv rotated to slot zero */

   VB->LastClipped = VB->FirstClipped;

   if (mask & 0x3f) {
      do { if (mask & 0x01) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*-1 + coord[idxPrev][1]*0 + coord[idxPrev][2]*0 + coord[idxPrev][3]*1; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*-1 + coord[idx][1]*0 + coord[idx][2]*0 + coord[idx][3]*1; clipmask[idxPrev] |= 0x01; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x01; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
      do { if (mask & 0x02) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*1 + coord[idxPrev][1]*0 + coord[idxPrev][2]*0 + coord[idxPrev][3]*1; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*1 + coord[idx][1]*0 + coord[idx][2]*0 + coord[idx][3]*1; clipmask[idxPrev] |= 0x02; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x02; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
      do { if (mask & 0x04) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*0 + coord[idxPrev][1]*-1 + coord[idxPrev][2]*0 + coord[idxPrev][3]*1; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*0 + coord[idx][1]*-1 + coord[idx][2]*0 + coord[idx][3]*1; clipmask[idxPrev] |= 0x04; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x04; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
      do { if (mask & 0x08) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*0 + coord[idxPrev][1]*1 + coord[idxPrev][2]*0 + coord[idxPrev][3]*1; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*0 + coord[idx][1]*1 + coord[idx][2]*0 + coord[idx][3]*1; clipmask[idxPrev] |= 0x08; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x08; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
      do { if (mask & 0x20) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*0 + coord[idxPrev][1]*0 + coord[idxPrev][2]*-1 + coord[idxPrev][3]*1; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*0 + coord[idx][1]*0 + coord[idx][2]*-1 + coord[idx][3]*1; clipmask[idxPrev] |= 0x20; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x20; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
      do { if (mask & 0x10) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*0 + coord[idxPrev][1]*0 + coord[idxPrev][2]*1 + coord[idxPrev][3]*1; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*0 + coord[idx][1]*0 + coord[idx][2]*1 + coord[idx][3]*1; clipmask[idxPrev] |= 0x10; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x10; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
   }

   if (mask & 0x40) {
      for (p=0;p<6;p++) {
         if (ctx->Transform.ClipPlanesEnabled & (1 << p)) {
            const GLfloat a = ctx->Transform._ClipUserPlane[p][0];
            const GLfloat b = ctx->Transform._ClipUserPlane[p][1];
            const GLfloat c = ctx->Transform._ClipUserPlane[p][2];
            const GLfloat d = ctx->Transform._ClipUserPlane[p][3];
            do { if (mask & 0x40) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*a + coord[idxPrev][1]*b + coord[idxPrev][2]*c + coord[idxPrev][3]*d; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*a + coord[idx][1]*b + coord[idx][2]*c + coord[idx][3]*d; clipmask[idxPrev] |= 0x40; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x40; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
         }
      }
   }

   if (ctx->_TriangleCaps & 0x1) {
      if (pv != inlist[0]) {
	 ;
	 tnl->Driver.Render.CopyPV( ctx, inlist[0], pv );
      }
   }

   tnl->Driver.Render.ClippedPolygon( ctx, inlist, n );
}


/* Clip a quad against the viewport and user clip planes.
 */
static  void
clip_quad_4( GLcontext *ctx, GLuint v0, GLuint v1, GLuint v2, GLuint v3,
                GLubyte mask )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   interp_func interp = tnl->Driver.Render.Interp;
   GLfloat (*coord)[4] = VB->ClipPtr->data;
   GLuint pv = v3;
   GLuint vlist[2][((2 * (6 + 6))+1)];
   GLuint *inlist = vlist[0], *outlist = vlist[1];
   GLuint p;
   GLubyte *clipmask = VB->ClipMask;
   GLuint n = 4;

   do { inlist[0] = v3; inlist[1] = v0; inlist[2] = v1; inlist[3] = v2; } while(0); /* pv rotated to slot zero */

   VB->LastClipped = VB->FirstClipped;

   if (mask & 0x3f) {
      do { if (mask & 0x01) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*-1 + coord[idxPrev][1]*0 + coord[idxPrev][2]*0 + coord[idxPrev][3]*1; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*-1 + coord[idx][1]*0 + coord[idx][2]*0 + coord[idx][3]*1; clipmask[idxPrev] |= 0x01; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x01; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
      do { if (mask & 0x02) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*1 + coord[idxPrev][1]*0 + coord[idxPrev][2]*0 + coord[idxPrev][3]*1; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*1 + coord[idx][1]*0 + coord[idx][2]*0 + coord[idx][3]*1; clipmask[idxPrev] |= 0x02; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x02; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
      do { if (mask & 0x04) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*0 + coord[idxPrev][1]*-1 + coord[idxPrev][2]*0 + coord[idxPrev][3]*1; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*0 + coord[idx][1]*-1 + coord[idx][2]*0 + coord[idx][3]*1; clipmask[idxPrev] |= 0x04; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x04; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
      do { if (mask & 0x08) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*0 + coord[idxPrev][1]*1 + coord[idxPrev][2]*0 + coord[idxPrev][3]*1; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*0 + coord[idx][1]*1 + coord[idx][2]*0 + coord[idx][3]*1; clipmask[idxPrev] |= 0x08; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x08; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
      do { if (mask & 0x20) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*0 + coord[idxPrev][1]*0 + coord[idxPrev][2]*-1 + coord[idxPrev][3]*1; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*0 + coord[idx][1]*0 + coord[idx][2]*-1 + coord[idx][3]*1; clipmask[idxPrev] |= 0x20; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x20; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
      do { if (mask & 0x10) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*0 + coord[idxPrev][1]*0 + coord[idxPrev][2]*1 + coord[idxPrev][3]*1; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*0 + coord[idx][1]*0 + coord[idx][2]*1 + coord[idx][3]*1; clipmask[idxPrev] |= 0x10; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x10; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
   }

   if (mask & 0x40) {
      for (p=0;p<6;p++) {
	 if (ctx->Transform.ClipPlanesEnabled & (1 << p)) {
            const GLfloat a = ctx->Transform._ClipUserPlane[p][0];
            const GLfloat b = ctx->Transform._ClipUserPlane[p][1];
            const GLfloat c = ctx->Transform._ClipUserPlane[p][2];
            const GLfloat d = ctx->Transform._ClipUserPlane[p][3];
	    do { if (mask & 0x40) { GLuint idxPrev = inlist[0]; GLfloat dpPrev = coord[idxPrev][0]*a + coord[idxPrev][1]*b + coord[idxPrev][2]*c + coord[idxPrev][3]*d; GLuint outcount = 0; GLuint i; inlist[n] = inlist[0]; for (i = 1; i <= n; i++) { GLuint idx = inlist[i]; GLfloat dp = coord[idx][0]*a + coord[idx][1]*b + coord[idx][2]*c + coord[idx][3]*d; clipmask[idxPrev] |= 0x40; if (!(((fi_type *) &(dpPrev))->i & (1<<31))) { outlist[outcount++] = idxPrev; clipmask[idxPrev] &= ~0x40; } if (((((fi_type *) &(dp))->i ^ ((fi_type *) &(dpPrev))->i) & (1<<31))) { GLuint newvert = VB->LastClipped++; VB->ClipMask[newvert] = 0; outlist[outcount++] = newvert; if ((((fi_type *) &(dp))->i & (1<<31))) { GLfloat t = dp / (dp - dpPrev); do { coord[newvert][0] = (((coord[idx])[0]) + ((t)) * (((coord[idxPrev])[0]) - ((coord[idx])[0]))); coord[newvert][1] = (((coord[idx])[1]) + ((t)) * (((coord[idxPrev])[1]) - ((coord[idx])[1]))); coord[newvert][2] = (((coord[idx])[2]) + ((t)) * (((coord[idxPrev])[2]) - ((coord[idx])[2]))); coord[newvert][3] = (((coord[idx])[3]) + ((t)) * (((coord[idxPrev])[3]) - ((coord[idx])[3]))); } while (0); interp( ctx, t, newvert, idx, idxPrev, 0x1 ); } else { GLfloat t = dpPrev / (dpPrev - dp); do { coord[newvert][0] = (((coord[idxPrev])[0]) + ((t)) * (((coord[idx])[0]) - ((coord[idxPrev])[0]))); coord[newvert][1] = (((coord[idxPrev])[1]) + ((t)) * (((coord[idx])[1]) - ((coord[idxPrev])[1]))); coord[newvert][2] = (((coord[idxPrev])[2]) + ((t)) * (((coord[idx])[2]) - ((coord[idxPrev])[2]))); coord[newvert][3] = (((coord[idxPrev])[3]) + ((t)) * (((coord[idx])[3]) - ((coord[idxPrev])[3]))); } while (0); interp( ctx, t, newvert, idxPrev, idx, 0x0 ); } } idxPrev = idx; dpPrev = dp; } if (outcount < 3) return; { GLuint *tmp = inlist; inlist = outlist; outlist = tmp; n = outcount; } } } while (0);
	 }
      }
   }

   if (ctx->_TriangleCaps & 0x1) {
      if (pv != inlist[0]) {
	 ;
	 tnl->Driver.Render.CopyPV( ctx, inlist[0], pv );
      }
   }

   tnl->Driver.Render.ClippedPolygon( ctx, inlist, n );
}




/**********************************************************************/
/*              Clip and render whole begin/end objects               */
/**********************************************************************/



/* Vertices, with the possibility of clipping.
 */



static void clip_render_points_verts( GLcontext *ctx,
				GLuint start,
				GLuint count,
				GLuint flags )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0000 );
   tnl->Driver.Render.Points( ctx, start, count );
   ;
}

static void clip_render_lines_verts( GLcontext *ctx,
			       GLuint start,
			       GLuint count,
			       GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0001 );
   for (j=start+1; j<count; j+=2 ) {
      if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
      do { GLubyte c1 = mask[j-1], c2 = mask[j]; GLubyte ormask = c1|c2; if (!ormask) LineFunc( ctx, j-1, j ); else if (!(c1 & c2 & 0x3f)) clip_line_4( ctx, j-1, j, ormask ); } while (0);
   }
   ;
}


static void clip_render_line_strip_verts( GLcontext *ctx,
				    GLuint start,
				    GLuint count,
				    GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0003 );

   if ((flags & 0x100)) {
      if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
   }

   for (j=start+1; j<count; j++ )
      do { GLubyte c1 = mask[j-1], c2 = mask[j]; GLubyte ormask = c1|c2; if (!ormask) LineFunc( ctx, j-1, j ); else if (!(c1 & c2 & 0x3f)) clip_line_4( ctx, j-1, j, ormask ); } while (0);

   ;
}


static void clip_render_line_loop_verts( GLcontext *ctx,
				   GLuint start,
				   GLuint count,
				   GLuint flags )
{
   GLuint i;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;

   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0002 );

   if (start+1 < count) {
      if ((flags & 0x100)) {
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 do { GLubyte c1 = mask[start], c2 = mask[start+1]; GLubyte ormask = c1|c2; if (!ormask) LineFunc( ctx, start, start+1 ); else if (!(c1 & c2 & 0x3f)) clip_line_4( ctx, start, start+1, ormask ); } while (0);
      }

      for ( i = start+2 ; i < count ; i++) {
	 do { GLubyte c1 = mask[i-1], c2 = mask[i]; GLubyte ormask = c1|c2; if (!ormask) LineFunc( ctx, i-1, i ); else if (!(c1 & c2 & 0x3f)) clip_line_4( ctx, i-1, i, ormask ); } while (0);
      }

      if ( (flags & 0x200)) {
	 do { GLubyte c1 = mask[count-1], c2 = mask[start]; GLubyte ormask = c1|c2; if (!ormask) LineFunc( ctx, count-1, start ); else if (!(c1 & c2 & 0x3f)) clip_line_4( ctx, count-1, start, ormask ); } while (0);
      }
   }

   ;
}


static void clip_render_triangles_verts( GLcontext *ctx,
				   GLuint start,
				   GLuint count,
				   GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0004 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+2; j<count; j+=3) {
	 /* Leave the edgeflags as supplied by the user.
	  */
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 do { GLubyte c1 = mask[j-2], c2 = mask[j-1], c3 = mask[j]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, j-2, j-1, j ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, j-2, j-1, j, ormask ); } while (0);
      }
   } else {
      for (j=start+2; j<count; j+=3) {
	 do { GLubyte c1 = mask[j-2], c2 = mask[j-1], c3 = mask[j]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, j-2, j-1, j ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, j-2, j-1, j, ormask ); } while (0);
      }
   }
   ;
}



static void clip_render_tri_strip_verts( GLcontext *ctx,
				   GLuint start,
				   GLuint count,
				   GLuint flags )
{
   GLuint j;
   GLuint parity = 0;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;

   if ((flags & 0x400))
      parity = 1;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0005 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+2;j<count;j++,parity^=1) {
	 GLuint ej2 = j-2+parity;
	 GLuint ej1 = j-1-parity;
	 GLuint ej = j;
	 GLboolean ef2 = VB->EdgeFlag[ej2];
	 GLboolean ef1 = VB->EdgeFlag[ej1];
	 GLboolean ef = VB->EdgeFlag[ej];
	 if ((flags & 0x100)) {
	    if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 }
	 VB->EdgeFlag[ej2] = 0x1;
	 VB->EdgeFlag[ej1] = 0x1;
	 VB->EdgeFlag[ej] = 0x1;
	 do { GLubyte c1 = mask[ej2], c2 = mask[ej1], c3 = mask[ej]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, ej2, ej1, ej ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, ej2, ej1, ej, ormask ); } while (0);
	 VB->EdgeFlag[ej2] = ef2;
	 VB->EdgeFlag[ej1] = ef1;
	 VB->EdgeFlag[ej] = ef;
      }
   } else {
      for (j=start+2; j<count ; j++, parity^=1) {
	 do { GLubyte c1 = mask[j-2+parity], c2 = mask[j-1-parity], c3 = mask[j]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, j-2+parity, j-1-parity, j ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, j-2+parity, j-1-parity, j, ormask ); } while (0);
      }
   }
   ;
}


static void clip_render_tri_fan_verts( GLcontext *ctx,
				 GLuint start,
				 GLuint count,
				 GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0006 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+2;j<count;j++) {
	 /* For trifans, all edges are boundary.
	  */
	 GLuint ejs = start;
	 GLuint ej1 = j-1;
	 GLuint ej = j;
	 GLboolean efs = VB->EdgeFlag[ejs];
	 GLboolean ef1 = VB->EdgeFlag[ej1];
	 GLboolean ef = VB->EdgeFlag[ej];
	 if ((flags & 0x100)) {
	    if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 }
	 VB->EdgeFlag[ejs] = 0x1;
	 VB->EdgeFlag[ej1] = 0x1;
	 VB->EdgeFlag[ej] = 0x1;
	 do { GLubyte c1 = mask[ejs], c2 = mask[ej1], c3 = mask[ej]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, ejs, ej1, ej ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, ejs, ej1, ej, ormask ); } while (0);
	 VB->EdgeFlag[ejs] = efs;
	 VB->EdgeFlag[ej1] = ef1;
	 VB->EdgeFlag[ej] = ef;
      }
   } else {
      for (j=start+2;j<count;j++) {
	 do { GLubyte c1 = mask[start], c2 = mask[j-1], c3 = mask[j]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, start, j-1, j ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, start, j-1, j, ormask ); } while (0);
      }
   }

   ;
}


static void clip_render_poly_verts( GLcontext *ctx,
			      GLuint start,
			      GLuint count,
			      GLuint flags )
{
   GLuint j = start+2;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0009 );
   if ((ctx->_TriangleCaps & 0x10)) {
      GLboolean efstart = VB->EdgeFlag[start];
      GLboolean efcount = VB->EdgeFlag[count-1];

      /* If the primitive does not begin here, the first edge
       * is non-boundary.
       */
      if (!(flags & 0x100))
	 VB->EdgeFlag[start] = 0x0;
      else {
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
      }

      /* If the primitive does not end here, the final edge is
       * non-boundary.
       */
      if (!(flags & 0x200))
	 VB->EdgeFlag[count-1] = 0x0;

      /* Draw the first triangles (possibly zero)
       */
      if (j+1<count) {
	 GLboolean ef = VB->EdgeFlag[j];
	 VB->EdgeFlag[j] = 0x0;
	 do { GLubyte c1 = mask[j-1], c2 = mask[j], c3 = mask[start]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, j-1, j, start ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, j-1, j, start, ormask ); } while (0);
	 VB->EdgeFlag[j] = ef;
	 j++;

	 /* Don't render the first edge again:
	  */
	 VB->EdgeFlag[start] = 0x0;

	 for (;j+1<count;j++) {
	    GLboolean efj = VB->EdgeFlag[j];
	    VB->EdgeFlag[j] = 0x0;
	    do { GLubyte c1 = mask[j-1], c2 = mask[j], c3 = mask[start]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, j-1, j, start ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, j-1, j, start, ormask ); } while (0);
	    VB->EdgeFlag[j] = efj;
	 }
      }

      /* Draw the last or only triangle
       */
      if (j < count)
	 do { GLubyte c1 = mask[j-1], c2 = mask[j], c3 = mask[start]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, j-1, j, start ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, j-1, j, start, ormask ); } while (0);

      /* Restore the first and last edgeflags:
       */
      VB->EdgeFlag[count-1] = efcount;
      VB->EdgeFlag[start] = efstart;

   }
   else {
      for (j=start+2;j<count;j++) {
	 do { GLubyte c1 = mask[j-1], c2 = mask[j], c3 = mask[start]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, j-1, j, start ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, j-1, j, start, ormask ); } while (0);
      }
   }
   ;
}

static void clip_render_quads_verts( GLcontext *ctx,
			       GLuint start,
			       GLuint count,
			       GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0007 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+3; j<count; j+=4) {
	 /* Use user-specified edgeflags for quads.
	  */
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 do { GLubyte c1 = mask[j-3], c2 = mask[j-2]; GLubyte c3 = mask[j-1], c4 = mask[j]; GLubyte ormask = c1|c2|c3|c4; if (!ormask) QuadFunc( ctx, j-3, j-2, j-1, j ); else if (!(c1 & c2 & c3 & c4 & 0x3f)) clip_quad_4( ctx, j-3, j-2, j-1, j, ormask ); } while (0);
      }
   } else {
      for (j=start+3; j<count; j+=4) {
	 do { GLubyte c1 = mask[j-3], c2 = mask[j-2]; GLubyte c3 = mask[j-1], c4 = mask[j]; GLubyte ormask = c1|c2|c3|c4; if (!ormask) QuadFunc( ctx, j-3, j-2, j-1, j ); else if (!(c1 & c2 & c3 & c4 & 0x3f)) clip_quad_4( ctx, j-3, j-2, j-1, j, ormask ); } while (0);
      }
   }
   ;
}

static void clip_render_quad_strip_verts( GLcontext *ctx,
				    GLuint start,
				    GLuint count,
				    GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0008 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+3;j<count;j+=2) {
	 /* All edges are boundary.  Set edgeflags to 1, draw the
	  * quad, and restore them to the original values.
	  */
	 GLboolean ef3 = VB->EdgeFlag[j-3];
	 GLboolean ef2 = VB->EdgeFlag[j-2];
	 GLboolean ef1 = VB->EdgeFlag[j-1];
	 GLboolean ef = VB->EdgeFlag[j];
	 if ((flags & 0x100)) {
	    if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 }
	 VB->EdgeFlag[j-3] = 0x1;
	 VB->EdgeFlag[j-2] = 0x1;
	 VB->EdgeFlag[j-1] = 0x1;
	 VB->EdgeFlag[j] = 0x1;
	 do { GLubyte c1 = mask[j-1], c2 = mask[j-3]; GLubyte c3 = mask[j-2], c4 = mask[j]; GLubyte ormask = c1|c2|c3|c4; if (!ormask) QuadFunc( ctx, j-1, j-3, j-2, j ); else if (!(c1 & c2 & c3 & c4 & 0x3f)) clip_quad_4( ctx, j-1, j-3, j-2, j, ormask ); } while (0);
	 VB->EdgeFlag[j-3] = ef3;
	 VB->EdgeFlag[j-2] = ef2;
	 VB->EdgeFlag[j-1] = ef1;
	 VB->EdgeFlag[j] = ef;
      }
   } else {
      for (j=start+3;j<count;j+=2) {
	 do { GLubyte c1 = mask[j-1], c2 = mask[j-3]; GLubyte c3 = mask[j-2], c4 = mask[j]; GLubyte ormask = c1|c2|c3|c4; if (!ormask) QuadFunc( ctx, j-1, j-3, j-2, j ); else if (!(c1 & c2 & c3 & c4 & 0x3f)) clip_quad_4( ctx, j-1, j-3, j-2, j, ormask ); } while (0);
      }
   }
   ;
}

static void clip_render_noop_verts( GLcontext *ctx,
			      GLuint start,
			      GLuint count,
			      GLuint flags )
{
   (void)(ctx && start && count && flags);
}

static void (*clip_render_tab_verts[0x0009+2])(GLcontext *,
							   GLuint,
							   GLuint,
							   GLuint) =
{
   clip_render_points_verts,
   clip_render_lines_verts,
   clip_render_line_loop_verts,
   clip_render_line_strip_verts,
   clip_render_triangles_verts,
   clip_render_tri_strip_verts,
   clip_render_tri_fan_verts,
   clip_render_quads_verts,
   clip_render_quad_strip_verts,
   clip_render_poly_verts,
   clip_render_noop_verts,
};



/* Elts, with the possibility of clipping.
 */

static void clip_render_points_elts( GLcontext *ctx,
				GLuint start,
				GLuint count,
				GLuint flags )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0000 );
   tnl->Driver.Render.Points( ctx, start, count );
   ;
}

static void clip_render_lines_elts( GLcontext *ctx,
			       GLuint start,
			       GLuint count,
			       GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0001 );
   for (j=start+1; j<count; j+=2 ) {
      if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
      do { GLubyte c1 = mask[elt[j-1]], c2 = mask[elt[j]]; GLubyte ormask = c1|c2; if (!ormask) LineFunc( ctx, elt[j-1], elt[j] ); else if (!(c1 & c2 & 0x3f)) clip_line_4( ctx, elt[j-1], elt[j], ormask ); } while (0);
   }
   ;
}


static void clip_render_line_strip_elts( GLcontext *ctx,
				    GLuint start,
				    GLuint count,
				    GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0003 );

   if ((flags & 0x100)) {
      if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
   }

   for (j=start+1; j<count; j++ )
      do { GLubyte c1 = mask[elt[j-1]], c2 = mask[elt[j]]; GLubyte ormask = c1|c2; if (!ormask) LineFunc( ctx, elt[j-1], elt[j] ); else if (!(c1 & c2 & 0x3f)) clip_line_4( ctx, elt[j-1], elt[j], ormask ); } while (0);

   ;
}


static void clip_render_line_loop_elts( GLcontext *ctx,
				   GLuint start,
				   GLuint count,
				   GLuint flags )
{
   GLuint i;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;

   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0002 );

   if (start+1 < count) {
      if ((flags & 0x100)) {
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 do { GLubyte c1 = mask[elt[start]], c2 = mask[elt[start+1]]; GLubyte ormask = c1|c2; if (!ormask) LineFunc( ctx, elt[start], elt[start+1] ); else if (!(c1 & c2 & 0x3f)) clip_line_4( ctx, elt[start], elt[start+1], ormask ); } while (0);
      }

      for ( i = start+2 ; i < count ; i++) {
	 do { GLubyte c1 = mask[elt[i-1]], c2 = mask[elt[i]]; GLubyte ormask = c1|c2; if (!ormask) LineFunc( ctx, elt[i-1], elt[i] ); else if (!(c1 & c2 & 0x3f)) clip_line_4( ctx, elt[i-1], elt[i], ormask ); } while (0);
      }

      if ( (flags & 0x200)) {
	 do { GLubyte c1 = mask[elt[count-1]], c2 = mask[elt[start]]; GLubyte ormask = c1|c2; if (!ormask) LineFunc( ctx, elt[count-1], elt[start] ); else if (!(c1 & c2 & 0x3f)) clip_line_4( ctx, elt[count-1], elt[start], ormask ); } while (0);
      }
   }

   ;
}


static void clip_render_triangles_elts( GLcontext *ctx,
				   GLuint start,
				   GLuint count,
				   GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0004 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+2; j<count; j+=3) {
	 /* Leave the edgeflags as supplied by the user.
	  */
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 do { GLubyte c1 = mask[elt[j-2]], c2 = mask[elt[j-1]], c3 = mask[elt[j]]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, elt[j-2], elt[j-1], elt[j] ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, elt[j-2], elt[j-1], elt[j], ormask ); } while (0);
      }
   } else {
      for (j=start+2; j<count; j+=3) {
	 do { GLubyte c1 = mask[elt[j-2]], c2 = mask[elt[j-1]], c3 = mask[elt[j]]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, elt[j-2], elt[j-1], elt[j] ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, elt[j-2], elt[j-1], elt[j], ormask ); } while (0);
      }
   }
   ;
}



static void clip_render_tri_strip_elts( GLcontext *ctx,
				   GLuint start,
				   GLuint count,
				   GLuint flags )
{
   GLuint j;
   GLuint parity = 0;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;

   if ((flags & 0x400))
      parity = 1;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0005 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+2;j<count;j++,parity^=1) {
	 GLuint ej2 = elt[j-2+parity];
	 GLuint ej1 = elt[j-1-parity];
	 GLuint ej = elt[j];
	 GLboolean ef2 = VB->EdgeFlag[ej2];
	 GLboolean ef1 = VB->EdgeFlag[ej1];
	 GLboolean ef = VB->EdgeFlag[ej];
	 if ((flags & 0x100)) {
	    if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 }
	 VB->EdgeFlag[ej2] = 0x1;
	 VB->EdgeFlag[ej1] = 0x1;
	 VB->EdgeFlag[ej] = 0x1;
	 do { GLubyte c1 = mask[ej2], c2 = mask[ej1], c3 = mask[ej]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, ej2, ej1, ej ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, ej2, ej1, ej, ormask ); } while (0);
	 VB->EdgeFlag[ej2] = ef2;
	 VB->EdgeFlag[ej1] = ef1;
	 VB->EdgeFlag[ej] = ef;
      }
   } else {
      for (j=start+2; j<count ; j++, parity^=1) {
	 do { GLubyte c1 = mask[elt[j-2+parity]], c2 = mask[elt[j-1-parity]], c3 = mask[elt[j]]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, elt[j-2+parity], elt[j-1-parity], elt[j] ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, elt[j-2+parity], elt[j-1-parity], elt[j], ormask ); } while (0);
      }
   }
   ;
}


static void clip_render_tri_fan_elts( GLcontext *ctx,
				 GLuint start,
				 GLuint count,
				 GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0006 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+2;j<count;j++) {
	 /* For trifans, all edges are boundary.
	  */
	 GLuint ejs = elt[start];
	 GLuint ej1 = elt[j-1];
	 GLuint ej = elt[j];
	 GLboolean efs = VB->EdgeFlag[ejs];
	 GLboolean ef1 = VB->EdgeFlag[ej1];
	 GLboolean ef = VB->EdgeFlag[ej];
	 if ((flags & 0x100)) {
	    if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 }
	 VB->EdgeFlag[ejs] = 0x1;
	 VB->EdgeFlag[ej1] = 0x1;
	 VB->EdgeFlag[ej] = 0x1;
	 do { GLubyte c1 = mask[ejs], c2 = mask[ej1], c3 = mask[ej]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, ejs, ej1, ej ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, ejs, ej1, ej, ormask ); } while (0);
	 VB->EdgeFlag[ejs] = efs;
	 VB->EdgeFlag[ej1] = ef1;
	 VB->EdgeFlag[ej] = ef;
      }
   } else {
      for (j=start+2;j<count;j++) {
	 do { GLubyte c1 = mask[elt[start]], c2 = mask[elt[j-1]], c3 = mask[elt[j]]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, elt[start], elt[j-1], elt[j] ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, elt[start], elt[j-1], elt[j], ormask ); } while (0);
      }
   }

   ;
}


static void clip_render_poly_elts( GLcontext *ctx,
			      GLuint start,
			      GLuint count,
			      GLuint flags )
{
   GLuint j = start+2;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0009 );
   if ((ctx->_TriangleCaps & 0x10)) {
      GLboolean efstart = VB->EdgeFlag[elt[start]];
      GLboolean efcount = VB->EdgeFlag[elt[count-1]];

      /* If the primitive does not begin here, the first edge
       * is non-boundary.
       */
      if (!(flags & 0x100))
	 VB->EdgeFlag[elt[start]] = 0x0;
      else {
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
      }

      /* If the primitive does not end here, the final edge is
       * non-boundary.
       */
      if (!(flags & 0x200))
	 VB->EdgeFlag[elt[count-1]] = 0x0;

      /* Draw the first triangles (possibly zero)
       */
      if (j+1<count) {
	 GLboolean ef = VB->EdgeFlag[elt[j]];
	 VB->EdgeFlag[elt[j]] = 0x0;
	 do { GLubyte c1 = mask[elt[j-1]], c2 = mask[elt[j]], c3 = mask[elt[start]]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, elt[j-1], elt[j], elt[start] ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, elt[j-1], elt[j], elt[start], ormask ); } while (0);
	 VB->EdgeFlag[elt[j]] = ef;
	 j++;

	 /* Don't render the first edge again:
	  */
	 VB->EdgeFlag[elt[start]] = 0x0;

	 for (;j+1<count;j++) {
	    GLboolean efj = VB->EdgeFlag[elt[j]];
	    VB->EdgeFlag[elt[j]] = 0x0;
	    do { GLubyte c1 = mask[elt[j-1]], c2 = mask[elt[j]], c3 = mask[elt[start]]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, elt[j-1], elt[j], elt[start] ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, elt[j-1], elt[j], elt[start], ormask ); } while (0);
	    VB->EdgeFlag[elt[j]] = efj;
	 }
      }

      /* Draw the last or only triangle
       */
      if (j < count)
	 do { GLubyte c1 = mask[elt[j-1]], c2 = mask[elt[j]], c3 = mask[elt[start]]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, elt[j-1], elt[j], elt[start] ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, elt[j-1], elt[j], elt[start], ormask ); } while (0);

      /* Restore the first and last edgeflags:
       */
      VB->EdgeFlag[elt[count-1]] = efcount;
      VB->EdgeFlag[elt[start]] = efstart;

   }
   else {
      for (j=start+2;j<count;j++) {
	 do { GLubyte c1 = mask[elt[j-1]], c2 = mask[elt[j]], c3 = mask[elt[start]]; GLubyte ormask = c1|c2|c3; if (!ormask) TriangleFunc( ctx, elt[j-1], elt[j], elt[start] ); else if (!(c1 & c2 & c3 & 0x3f)) clip_tri_4( ctx, elt[j-1], elt[j], elt[start], ormask ); } while (0);
      }
   }
   ;
}

static void clip_render_quads_elts( GLcontext *ctx,
			       GLuint start,
			       GLuint count,
			       GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0007 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+3; j<count; j+=4) {
	 /* Use user-specified edgeflags for quads.
	  */
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 do { GLubyte c1 = mask[elt[j-3]], c2 = mask[elt[j-2]]; GLubyte c3 = mask[elt[j-1]], c4 = mask[elt[j]]; GLubyte ormask = c1|c2|c3|c4; if (!ormask) QuadFunc( ctx, elt[j-3], elt[j-2], elt[j-1], elt[j] ); else if (!(c1 & c2 & c3 & c4 & 0x3f)) clip_quad_4( ctx, elt[j-3], elt[j-2], elt[j-1], elt[j], ormask ); } while (0);
      }
   } else {
      for (j=start+3; j<count; j+=4) {
	 do { GLubyte c1 = mask[elt[j-3]], c2 = mask[elt[j-2]]; GLubyte c3 = mask[elt[j-1]], c4 = mask[elt[j]]; GLubyte ormask = c1|c2|c3|c4; if (!ormask) QuadFunc( ctx, elt[j-3], elt[j-2], elt[j-1], elt[j] ); else if (!(c1 & c2 & c3 & c4 & 0x3f)) clip_quad_4( ctx, elt[j-3], elt[j-2], elt[j-1], elt[j], ormask ); } while (0);
      }
   }
   ;
}

static void clip_render_quad_strip_elts( GLcontext *ctx,
				    GLuint start,
				    GLuint count,
				    GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const GLubyte *mask = VB->ClipMask; const GLuint sz = VB->ClipPtr->size; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) mask; (void) sz; (void) stipple;;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0008 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+3;j<count;j+=2) {
	 /* All edges are boundary.  Set edgeflags to 1, draw the
	  * quad, and restore them to the original values.
	  */
	 GLboolean ef3 = VB->EdgeFlag[elt[j-3]];
	 GLboolean ef2 = VB->EdgeFlag[elt[j-2]];
	 GLboolean ef1 = VB->EdgeFlag[elt[j-1]];
	 GLboolean ef = VB->EdgeFlag[elt[j]];
	 if ((flags & 0x100)) {
	    if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 }
	 VB->EdgeFlag[elt[j-3]] = 0x1;
	 VB->EdgeFlag[elt[j-2]] = 0x1;
	 VB->EdgeFlag[elt[j-1]] = 0x1;
	 VB->EdgeFlag[elt[j]] = 0x1;
	 do { GLubyte c1 = mask[elt[j-1]], c2 = mask[elt[j-3]]; GLubyte c3 = mask[elt[j-2]], c4 = mask[elt[j]]; GLubyte ormask = c1|c2|c3|c4; if (!ormask) QuadFunc( ctx, elt[j-1], elt[j-3], elt[j-2], elt[j] ); else if (!(c1 & c2 & c3 & c4 & 0x3f)) clip_quad_4( ctx, elt[j-1], elt[j-3], elt[j-2], elt[j], ormask ); } while (0);
	 VB->EdgeFlag[elt[j-3]] = ef3;
	 VB->EdgeFlag[elt[j-2]] = ef2;
	 VB->EdgeFlag[elt[j-1]] = ef1;
	 VB->EdgeFlag[elt[j]] = ef;
      }
   } else {
      for (j=start+3;j<count;j+=2) {
	 do { GLubyte c1 = mask[elt[j-1]], c2 = mask[elt[j-3]]; GLubyte c3 = mask[elt[j-2]], c4 = mask[elt[j]]; GLubyte ormask = c1|c2|c3|c4; if (!ormask) QuadFunc( ctx, elt[j-1], elt[j-3], elt[j-2], elt[j] ); else if (!(c1 & c2 & c3 & c4 & 0x3f)) clip_quad_4( ctx, elt[j-1], elt[j-3], elt[j-2], elt[j], ormask ); } while (0);
      }
   }
   ;
}

static void clip_render_noop_elts( GLcontext *ctx,
			      GLuint start,
			      GLuint count,
			      GLuint flags )
{
   (void)(ctx && start && count && flags);
}

static void (*clip_render_tab_elts[0x0009+2])(GLcontext *,
							   GLuint,
							   GLuint,
							   GLuint) =
{
   clip_render_points_elts,
   clip_render_lines_elts,
   clip_render_line_loop_elts,
   clip_render_line_strip_elts,
   clip_render_triangles_elts,
   clip_render_tri_strip_elts,
   clip_render_tri_fan_elts,
   clip_render_quads_elts,
   clip_render_quad_strip_elts,
   clip_render_poly_elts,
   clip_render_noop_elts,
};






/* TODO: do this for all primitives, verts and elts:
 */
static void clip_elt_triangles( GLcontext *ctx,
				GLuint start,
				GLuint count,
				GLuint flags )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   render_func render_tris = tnl->Driver.Render.PrimTabElts[0x0004];
   struct vertex_buffer *VB = &tnl->vb;
   const GLuint * const elt = VB->Elts;
   GLubyte *mask = VB->ClipMask;
   GLuint last = count-2;
   GLuint j;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0004 );

   for (j=start; j < last; j+=3 ) {
      GLubyte c1 = mask[elt[j]];
      GLubyte c2 = mask[elt[j+1]];
      GLubyte c3 = mask[elt[j+2]];
      GLubyte ormask = c1|c2|c3;
      if (ormask) {
	 if (start < j)
	    render_tris( ctx, start, j, 0 );
	 if (!(c1&c2&c3&0x3f))
	    clip_tri_4( ctx, elt[j], elt[j+1], elt[j+2], ormask );
	 start = j+3;
      }
   }

   if (start < j)
      render_tris( ctx, start, j, 0 );
}

/**********************************************************************/
/*                  Render whole begin/end objects                    */
/**********************************************************************/



/* Vertices, no clipping.
 */


/* $Id: t_vb_rendertmp.h,v 1.10 2002/10/29 20:29:04 brianp Exp $ */


static void _tnl_render_points_verts( GLcontext *ctx,
				GLuint start,
				GLuint count,
				GLuint flags )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0000 );
   tnl->Driver.Render.Points( ctx, start, count );
   ;
}

static void _tnl_render_lines_verts( GLcontext *ctx,
			       GLuint start,
			       GLuint count,
			       GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0001 );
   for (j=start+1; j<count; j+=2 ) {
      if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
      LineFunc( ctx, j-1, j );
   }
   ;
}


static void _tnl_render_line_strip_verts( GLcontext *ctx,
				    GLuint start,
				    GLuint count,
				    GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0003 );

   if ((flags & 0x100)) {
      if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
   }

   for (j=start+1; j<count; j++ )
      LineFunc( ctx, j-1, j );

   ;
}


static void _tnl_render_line_loop_verts( GLcontext *ctx,
				   GLuint start,
				   GLuint count,
				   GLuint flags )
{
   GLuint i;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;

   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0002 );

   if (start+1 < count) {
      if ((flags & 0x100)) {
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 LineFunc( ctx, start, start+1 );
      }

      for ( i = start+2 ; i < count ; i++) {
	 LineFunc( ctx, i-1, i );
      }

      if ( (flags & 0x200)) {
	 LineFunc( ctx, count-1, start );
      }
   }

   ;
}


static void _tnl_render_triangles_verts( GLcontext *ctx,
				   GLuint start,
				   GLuint count,
				   GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0004 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+2; j<count; j+=3) {
	 /* Leave the edgeflags as supplied by the user.
	  */
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 TriangleFunc( ctx, j-2, j-1, j );
      }
   } else {
      for (j=start+2; j<count; j+=3) {
	 TriangleFunc( ctx, j-2, j-1, j );
      }
   }
   ;
}



static void _tnl_render_tri_strip_verts( GLcontext *ctx,
				   GLuint start,
				   GLuint count,
				   GLuint flags )
{
   GLuint j;
   GLuint parity = 0;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;

   if ((flags & 0x400))
      parity = 1;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0005 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+2;j<count;j++,parity^=1) {
	 GLuint ej2 = j-2+parity;
	 GLuint ej1 = j-1-parity;
	 GLuint ej = j;
	 GLboolean ef2 = VB->EdgeFlag[ej2];
	 GLboolean ef1 = VB->EdgeFlag[ej1];
	 GLboolean ef = VB->EdgeFlag[ej];
	 if ((flags & 0x100)) {
	    if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 }
	 VB->EdgeFlag[ej2] = 0x1;
	 VB->EdgeFlag[ej1] = 0x1;
	 VB->EdgeFlag[ej] = 0x1;
	 TriangleFunc( ctx, ej2, ej1, ej );
	 VB->EdgeFlag[ej2] = ef2;
	 VB->EdgeFlag[ej1] = ef1;
	 VB->EdgeFlag[ej] = ef;
      }
   } else {
      for (j=start+2; j<count ; j++, parity^=1) {
	 TriangleFunc( ctx, j-2+parity, j-1-parity, j );
      }
   }
   ;
}


static void _tnl_render_tri_fan_verts( GLcontext *ctx,
				 GLuint start,
				 GLuint count,
				 GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0006 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+2;j<count;j++) {
	 /* For trifans, all edges are boundary.
	  */
	 GLuint ejs = start;
	 GLuint ej1 = j-1;
	 GLuint ej = j;
	 GLboolean efs = VB->EdgeFlag[ejs];
	 GLboolean ef1 = VB->EdgeFlag[ej1];
	 GLboolean ef = VB->EdgeFlag[ej];
	 if ((flags & 0x100)) {
	    if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 }
	 VB->EdgeFlag[ejs] = 0x1;
	 VB->EdgeFlag[ej1] = 0x1;
	 VB->EdgeFlag[ej] = 0x1;
	 TriangleFunc( ctx, ejs, ej1, ej );
	 VB->EdgeFlag[ejs] = efs;
	 VB->EdgeFlag[ej1] = ef1;
	 VB->EdgeFlag[ej] = ef;
      }
   } else {
      for (j=start+2;j<count;j++) {
	 TriangleFunc( ctx, start, j-1, j );
      }
   }

   ;
}


static void _tnl_render_poly_verts( GLcontext *ctx,
			      GLuint start,
			      GLuint count,
			      GLuint flags )
{
   GLuint j = start+2;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0009 );
   if ((ctx->_TriangleCaps & 0x10)) {
      GLboolean efstart = VB->EdgeFlag[start];
      GLboolean efcount = VB->EdgeFlag[count-1];

      /* If the primitive does not begin here, the first edge
       * is non-boundary.
       */
      if (!(flags & 0x100))
	 VB->EdgeFlag[start] = 0x0;
      else {
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
      }

      /* If the primitive does not end here, the final edge is
       * non-boundary.
       */
      if (!(flags & 0x200))
	 VB->EdgeFlag[count-1] = 0x0;

      /* Draw the first triangles (possibly zero)
       */
      if (j+1<count) {
	 GLboolean ef = VB->EdgeFlag[j];
	 VB->EdgeFlag[j] = 0x0;
	 TriangleFunc( ctx, j-1, j, start );
	 VB->EdgeFlag[j] = ef;
	 j++;

	 /* Don't render the first edge again:
	  */
	 VB->EdgeFlag[start] = 0x0;

	 for (;j+1<count;j++) {
	    GLboolean efj = VB->EdgeFlag[j];
	    VB->EdgeFlag[j] = 0x0;
	    TriangleFunc( ctx, j-1, j, start );
	    VB->EdgeFlag[j] = efj;
	 }
      }

      /* Draw the last or only triangle
       */
      if (j < count)
	 TriangleFunc( ctx, j-1, j, start );

      /* Restore the first and last edgeflags:
       */
      VB->EdgeFlag[count-1] = efcount;
      VB->EdgeFlag[start] = efstart;

   }
   else {
      for (j=start+2;j<count;j++) {
	 TriangleFunc( ctx, j-1, j, start );
      }
   }
   ;
}

static void _tnl_render_quads_verts( GLcontext *ctx,
			       GLuint start,
			       GLuint count,
			       GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0007 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+3; j<count; j+=4) {
	 /* Use user-specified edgeflags for quads.
	  */
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 QuadFunc( ctx, j-3, j-2, j-1, j );
      }
   } else {
      for (j=start+3; j<count; j+=4) {
	 QuadFunc( ctx, j-3, j-2, j-1, j );
      }
   }
   ;
}

static void _tnl_render_quad_strip_verts( GLcontext *ctx,
				    GLuint start,
				    GLuint count,
				    GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0008 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+3;j<count;j+=2) {
	 /* All edges are boundary.  Set edgeflags to 1, draw the
	  * quad, and restore them to the original values.
	  */
	 GLboolean ef3 = VB->EdgeFlag[j-3];
	 GLboolean ef2 = VB->EdgeFlag[j-2];
	 GLboolean ef1 = VB->EdgeFlag[j-1];
	 GLboolean ef = VB->EdgeFlag[j];
	 if ((flags & 0x100)) {
	    if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 }
	 VB->EdgeFlag[j-3] = 0x1;
	 VB->EdgeFlag[j-2] = 0x1;
	 VB->EdgeFlag[j-1] = 0x1;
	 VB->EdgeFlag[j] = 0x1;
	 QuadFunc( ctx, j-1, j-3, j-2, j );
	 VB->EdgeFlag[j-3] = ef3;
	 VB->EdgeFlag[j-2] = ef2;
	 VB->EdgeFlag[j-1] = ef1;
	 VB->EdgeFlag[j] = ef;
      }
   } else {
      for (j=start+3;j<count;j+=2) {
	 QuadFunc( ctx, j-1, j-3, j-2, j );
      }
   }
   ;
}

static void _tnl_render_noop_verts( GLcontext *ctx,
			      GLuint start,
			      GLuint count,
			      GLuint flags )
{
   (void)(ctx && start && count && flags);
}

 void (*_tnl_render_tab_verts[0x0009+2])(GLcontext *,
							   GLuint,
							   GLuint,
							   GLuint) =
{
   _tnl_render_points_verts,
   _tnl_render_lines_verts,
   _tnl_render_line_loop_verts,
   _tnl_render_line_strip_verts,
   _tnl_render_triangles_verts,
   _tnl_render_tri_strip_verts,
   _tnl_render_tri_fan_verts,
   _tnl_render_quads_verts,
   _tnl_render_quad_strip_verts,
   _tnl_render_poly_verts,
   _tnl_render_noop_verts,
};







/* Elts, no clipping.
 */
/* $Id: t_vb_rendertmp.h,v 1.10 2002/10/29 20:29:04 brianp Exp $ */


static void _tnl_render_points_elts( GLcontext *ctx,
				GLuint start,
				GLuint count,
				GLuint flags )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0000 );
   tnl->Driver.Render.Points( ctx, start, count );
   ;
}

static void _tnl_render_lines_elts( GLcontext *ctx,
			       GLuint start,
			       GLuint count,
			       GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0001 );
   for (j=start+1; j<count; j+=2 ) {
      if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
      LineFunc( ctx, elt[j-1], elt[j] );
   }
   ;
}


static void _tnl_render_line_strip_elts( GLcontext *ctx,
				    GLuint start,
				    GLuint count,
				    GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0003 );

   if ((flags & 0x100)) {
      if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
   }

   for (j=start+1; j<count; j++ )
      LineFunc( ctx, elt[j-1], elt[j] );

   ;
}


static void _tnl_render_line_loop_elts( GLcontext *ctx,
				   GLuint start,
				   GLuint count,
				   GLuint flags )
{
   GLuint i;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;

   (void) flags;

   ctx->OcclusionResult = 0x1;
   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0002 );

   if (start+1 < count) {
      if ((flags & 0x100)) {
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 LineFunc( ctx, elt[start], elt[start+1] );
      }

      for ( i = start+2 ; i < count ; i++) {
	 LineFunc( ctx, elt[i-1], elt[i] );
      }

      if ( (flags & 0x200)) {
	 LineFunc( ctx, elt[count-1], elt[start] );
      }
   }

   ;
}


static void _tnl_render_triangles_elts( GLcontext *ctx,
				   GLuint start,
				   GLuint count,
				   GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0004 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+2; j<count; j+=3) {
	 /* Leave the edgeflags as supplied by the user.
	  */
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 TriangleFunc( ctx, elt[j-2], elt[j-1], elt[j] );
      }
   } else {
      for (j=start+2; j<count; j+=3) {
	 TriangleFunc( ctx, elt[j-2], elt[j-1], elt[j] );
      }
   }
   ;
}



static void _tnl_render_tri_strip_elts( GLcontext *ctx,
				   GLuint start,
				   GLuint count,
				   GLuint flags )
{
   GLuint j;
   GLuint parity = 0;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;

   if ((flags & 0x400))
      parity = 1;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0005 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+2;j<count;j++,parity^=1) {
	 GLuint ej2 = elt[j-2+parity];
	 GLuint ej1 = elt[j-1-parity];
	 GLuint ej = elt[j];
	 GLboolean ef2 = VB->EdgeFlag[ej2];
	 GLboolean ef1 = VB->EdgeFlag[ej1];
	 GLboolean ef = VB->EdgeFlag[ej];
	 if ((flags & 0x100)) {
	    if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 }
	 VB->EdgeFlag[ej2] = 0x1;
	 VB->EdgeFlag[ej1] = 0x1;
	 VB->EdgeFlag[ej] = 0x1;
	 TriangleFunc( ctx, ej2, ej1, ej );
	 VB->EdgeFlag[ej2] = ef2;
	 VB->EdgeFlag[ej1] = ef1;
	 VB->EdgeFlag[ej] = ef;
      }
   } else {
      for (j=start+2; j<count ; j++, parity^=1) {
	 TriangleFunc( ctx, elt[j-2+parity], elt[j-1-parity], elt[j] );
      }
   }
   ;
}


static void _tnl_render_tri_fan_elts( GLcontext *ctx,
				 GLuint start,
				 GLuint count,
				 GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0006 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+2;j<count;j++) {
	 /* For trifans, all edges are boundary.
	  */
	 GLuint ejs = elt[start];
	 GLuint ej1 = elt[j-1];
	 GLuint ej = elt[j];
	 GLboolean efs = VB->EdgeFlag[ejs];
	 GLboolean ef1 = VB->EdgeFlag[ej1];
	 GLboolean ef = VB->EdgeFlag[ej];
	 if ((flags & 0x100)) {
	    if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 }
	 VB->EdgeFlag[ejs] = 0x1;
	 VB->EdgeFlag[ej1] = 0x1;
	 VB->EdgeFlag[ej] = 0x1;
	 TriangleFunc( ctx, ejs, ej1, ej );
	 VB->EdgeFlag[ejs] = efs;
	 VB->EdgeFlag[ej1] = ef1;
	 VB->EdgeFlag[ej] = ef;
      }
   } else {
      for (j=start+2;j<count;j++) {
	 TriangleFunc( ctx, elt[start], elt[j-1], elt[j] );
      }
   }

   ;
}


static void _tnl_render_poly_elts( GLcontext *ctx,
			      GLuint start,
			      GLuint count,
			      GLuint flags )
{
   GLuint j = start+2;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0009 );
   if ((ctx->_TriangleCaps & 0x10)) {
      GLboolean efstart = VB->EdgeFlag[elt[start]];
      GLboolean efcount = VB->EdgeFlag[elt[count-1]];

      /* If the primitive does not begin here, the first edge
       * is non-boundary.
       */
      if (!(flags & 0x100))
	 VB->EdgeFlag[elt[start]] = 0x0;
      else {
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
      }

      /* If the primitive does not end here, the final edge is
       * non-boundary.
       */
      if (!(flags & 0x200))
	 VB->EdgeFlag[elt[count-1]] = 0x0;

      /* Draw the first triangles (possibly zero)
       */
      if (j+1<count) {
	 GLboolean ef = VB->EdgeFlag[elt[j]];
	 VB->EdgeFlag[elt[j]] = 0x0;
	 TriangleFunc( ctx, elt[j-1], elt[j], elt[start] );
	 VB->EdgeFlag[elt[j]] = ef;
	 j++;

	 /* Don't render the first edge again:
	  */
	 VB->EdgeFlag[elt[start]] = 0x0;

	 for (;j+1<count;j++) {
	    GLboolean efj = VB->EdgeFlag[elt[j]];
	    VB->EdgeFlag[elt[j]] = 0x0;
	    TriangleFunc( ctx, elt[j-1], elt[j], elt[start] );
	    VB->EdgeFlag[elt[j]] = efj;
	 }
      }

      /* Draw the last or only triangle
       */
      if (j < count)
	 TriangleFunc( ctx, elt[j-1], elt[j], elt[start] );

      /* Restore the first and last edgeflags:
       */
      VB->EdgeFlag[elt[count-1]] = efcount;
      VB->EdgeFlag[elt[start]] = efstart;

   }
   else {
      for (j=start+2;j<count;j++) {
	 TriangleFunc( ctx, elt[j-1], elt[j], elt[start] );
      }
   }
   ;
}

static void _tnl_render_quads_elts( GLcontext *ctx,
			       GLuint start,
			       GLuint count,
			       GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0007 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+3; j<count; j+=4) {
	 /* Use user-specified edgeflags for quads.
	  */
	 if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 QuadFunc( ctx, elt[j-3], elt[j-2], elt[j-1], elt[j] );
      }
   } else {
      for (j=start+3; j<count; j+=4) {
	 QuadFunc( ctx, elt[j-3], elt[j-2], elt[j-1], elt[j] );
      }
   }
   ;
}

static void _tnl_render_quad_strip_elts( GLcontext *ctx,
				    GLuint start,
				    GLuint count,
				    GLuint flags )
{
   GLuint j;
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context)); struct vertex_buffer *VB = &tnl->vb; const GLuint * const elt = VB->Elts; const line_func LineFunc = tnl->Driver.Render.Line; const triangle_func TriangleFunc = tnl->Driver.Render.Triangle; const quad_func QuadFunc = tnl->Driver.Render.Quad; const GLboolean stipple = ctx->Line.StippleFlag; (void) (LineFunc && TriangleFunc && QuadFunc); (void) elt; (void) stipple;
   (void) flags;

   tnl->Driver.Render.PrimitiveNotify( ctx, 0x0008 );
   if ((ctx->_TriangleCaps & 0x10)) {
      for (j=start+3;j<count;j+=2) {
	 /* All edges are boundary.  Set edgeflags to 1, draw the
	  * quad, and restore them to the original values.
	  */
	 GLboolean ef3 = VB->EdgeFlag[elt[j-3]];
	 GLboolean ef2 = VB->EdgeFlag[elt[j-2]];
	 GLboolean ef1 = VB->EdgeFlag[elt[j-1]];
	 GLboolean ef = VB->EdgeFlag[elt[j]];
	 if ((flags & 0x100)) {
	    if (stipple) tnl->Driver.Render.ResetLineStipple( ctx );
	 }
	 VB->EdgeFlag[elt[j-3]] = 0x1;
	 VB->EdgeFlag[elt[j-2]] = 0x1;
	 VB->EdgeFlag[elt[j-1]] = 0x1;
	 VB->EdgeFlag[elt[j]] = 0x1;
	 QuadFunc( ctx, elt[j-1], elt[j-3], elt[j-2], elt[j] );
	 VB->EdgeFlag[elt[j-3]] = ef3;
	 VB->EdgeFlag[elt[j-2]] = ef2;
	 VB->EdgeFlag[elt[j-1]] = ef1;
	 VB->EdgeFlag[elt[j]] = ef;
      }
   } else {
      for (j=start+3;j<count;j+=2) {
	 QuadFunc( ctx, elt[j-1], elt[j-3], elt[j-2], elt[j] );
      }
   }
   ;
}

static void _tnl_render_noop_elts( GLcontext *ctx,
			      GLuint start,
			      GLuint count,
			      GLuint flags )
{
   (void)(ctx && start && count && flags);
}

 void (*_tnl_render_tab_elts[0x0009+2])(GLcontext *,
							   GLuint,
							   GLuint,
							   GLuint) =
{
   _tnl_render_points_elts,
   _tnl_render_lines_elts,
   _tnl_render_line_loop_elts,
   _tnl_render_line_strip_elts,
   _tnl_render_triangles_elts,
   _tnl_render_tri_strip_elts,
   _tnl_render_tri_fan_elts,
   _tnl_render_quads_elts,
   _tnl_render_quad_strip_elts,
   _tnl_render_poly_elts,
   _tnl_render_noop_elts,
};







/**********************************************************************/
/*              Helper functions for drivers                  */
/**********************************************************************/

void _tnl_RenderClippedPolygon( GLcontext *ctx, const GLuint *elts, GLuint n )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   GLuint *tmp = VB->Elts;

   VB->Elts = (GLuint *)elts;
   tnl->Driver.Render.PrimTabElts[0x0009]( ctx, 0, n, 0x100|0x200 );
   VB->Elts = tmp;
}

void _tnl_RenderClippedLine( GLcontext *ctx, GLuint ii, GLuint jj )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   tnl->Driver.Render.Line( ctx, ii, jj );
}



/**********************************************************************/
/*              Clip and render whole vertex buffers                  */
/**********************************************************************/


static GLboolean run_render( GLcontext *ctx,
			     struct gl_pipeline_stage *stage )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   GLuint new_inputs = stage->changed_inputs;
   render_func *tab;
   GLint pass = 0;

   /* Allow the drivers to lock before projected verts are built so
    * that window coordinates are guarenteed not to change before
    * rendering.
    */
   ;

   tnl->Driver.Render.Start( ctx );


   tnl->Driver.Render.BuildVertices( ctx, 0, VB->Count, new_inputs );

   if (VB->ClipOrMask) {
      tab = VB->Elts ? clip_render_tab_elts : clip_render_tab_verts;
      clip_render_tab_elts[0x0004] = clip_elt_triangles;
   }
   else {
      tab = (VB->Elts ? 
	     tnl->Driver.Render.PrimTabElts : 
	     tnl->Driver.Render.PrimTabVerts);
   }

   do
   {
      GLuint i, length, flags = 0;
      for (i = VB->FirstPrimitive ; !(flags & 0x800) ; i += length)
      {
	 flags = VB->Primitive[i];
	 length= VB->PrimitiveLength[i];
	 ;
	 ;

	 if (MESA_VERBOSE & VERBOSE_PRIMS)
	    _mesa_debug(0, "MESA prim %s %d..%d\n", 
		    _mesa_lookup_enum_by_nr(flags & 0xff), 
		    i, i+length);

	 if (length)
	    tab[flags & 0xff]( ctx, i, i + length, flags );
      }
   } while (tnl->Driver.Render.Multipass &&
	    tnl->Driver.Render.Multipass( ctx, ++pass ));


   tnl->Driver.Render.Finish( ctx );
/*     _swrast_flush(ctx); */
/*     usleep(1000000); */
   return 0x0;		/* finished the pipe */
}


/**********************************************************************/
/*                          Render pipeline stage                     */
/**********************************************************************/



/* Quite a bit of work involved in finding out the inputs for the
 * render stage.
 */
static void check_render( GLcontext *ctx, struct gl_pipeline_stage *stage )
{
   GLuint inputs = (1 << 25);
   GLuint i;

   if (ctx->Visual.rgbMode) {
      inputs |= (1 << 3);

      if (ctx->_TriangleCaps & 0x2)
	 inputs |= (1 << 4);

      if (ctx->Texture._EnabledUnits) {
	 for (i = 0 ; i < ctx->Const.MaxTextureUnits ; i++) {
	    if (ctx->Texture.Unit[i]._ReallyEnabled)
	       inputs |= (1 << (8 + (i)));
	 }
      }
   }
   else {
      inputs |= (1 << 6);
   }

   if (ctx->Point._Attenuated)
      inputs |= (1 << 27);

   /* How do drivers turn this off?
    */
   if (ctx->Fog.Enabled)
      inputs |= (1 << 5);

   if (ctx->_TriangleCaps & 0x10)
      inputs |= (1 << 7);

   if (ctx->RenderMode==0x1C01)
      inputs |= ((1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15));

   stage->inputs = inputs;
}




static void dtr( struct gl_pipeline_stage *stage )
{
}


const struct gl_pipeline_stage _tnl_render_stage =
{
   "render",			/* name */
   (0x1000000 |
    (0x400 | 0x100) |
    0x400 |
    0x40000|
    0x400|
    0x2000|
    0x100|
    0x4000 |
    0x800000),		/* re-check (new inputs, interp function) */
   0,				/* re-run (always runs) */
   0x1,			/* active? */
   0,				/* inputs (set in check_render) */
   0,				/* outputs */
   0,				/* changed_inputs */
   0,			/* private data */
   dtr,				/* destructor */
   check_render,		/* check */
   run_render			/* run */
};
