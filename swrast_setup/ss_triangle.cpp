/* $Id: ss_triangle.c,v 1.20 2002/10/29 22:25:57 brianp Exp $ */

#include "glheader.h"
#include "colormac.h"
#include "macros.h"
#include "mtypes.h"

#include "tnl/t_context.h"

#include "ss_triangle.h"
#include "ss_context.h"

#define SS_RGBA_BIT         0x1
#define SS_OFFSET_BIT      0x2
#define SS_TWOSIDE_BIT     0x4
#define SS_UNFILLED_BIT            0x8
#define SS_MAX_TRIFUNC      0x10

static triangle_func tri_tab[SS_MAX_TRIFUNC];
static quad_func     quad_tab[SS_MAX_TRIFUNC];



/* static  */
void
 _swsetup_render_line_tri( GLcontext *ctx,
                                     GLuint e0, GLuint e1, GLuint e2,
                                      GLuint facing )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLubyte *ef = VB->EdgeFlag;
   SWvertex *verts = swsetup->verts;
   SWvertex *v0 = &verts[e0];
   SWvertex *v1 = &verts[e1];
   SWvertex *v2 = &verts[e2];
   GLchan c[2][4];
   GLchan s[2][4];
   GLuint i[2];

   /* cull testing */
   if (ctx->Polygon.CullFlag) {
      if (facing == 1 && ctx->Polygon.CullFaceMode != 0x0404)
         return;
      if (facing == 0 && ctx->Polygon.CullFaceMode != 0x0405)
         return;
   }

   if (ctx->_TriangleCaps & 0x1) {
      do { (c[0])[0] = (v0->color)[0]; (c[0])[1] = (v0->color)[1]; (c[0])[2] = (v0->color)[2]; (c[0])[3] = (v0->color)[3]; } while (0);
      do { (c[1])[0] = (v1->color)[0]; (c[1])[1] = (v1->color)[1]; (c[1])[2] = (v1->color)[2]; (c[1])[3] = (v1->color)[3]; } while (0);
      do { (s[0])[0] = (v0->specular)[0]; (s[0])[1] = (v0->specular)[1]; (s[0])[2] = (v0->specular)[2]; (s[0])[3] = (v0->specular)[3]; } while (0);
      do { (s[1])[0] = (v1->specular)[0]; (s[1])[1] = (v1->specular)[1]; (s[1])[2] = (v1->specular)[2]; (s[1])[3] = (v1->specular)[3]; } while (0);
      i[0] = v0->index;
      i[1] = v1->index;

      do { (v0->color)[0] = (v2->color)[0]; (v0->color)[1] = (v2->color)[1]; (v0->color)[2] = (v2->color)[2]; (v0->color)[3] = (v2->color)[3]; } while (0);
      do { (v1->color)[0] = (v2->color)[0]; (v1->color)[1] = (v2->color)[1]; (v1->color)[2] = (v2->color)[2]; (v1->color)[3] = (v2->color)[3]; } while (0);
      do { (v0->specular)[0] = (v2->specular)[0]; (v0->specular)[1] = (v2->specular)[1]; (v0->specular)[2] = (v2->specular)[2]; (v0->specular)[3] = (v2->specular)[3]; } while (0);
      do { (v1->specular)[0] = (v2->specular)[0]; (v1->specular)[1] = (v2->specular)[1]; (v1->specular)[2] = (v2->specular)[2]; (v1->specular)[3] = (v2->specular)[3]; } while (0);
      v0->index = v2->index;
      v1->index = v2->index;
   }

   if (swsetup->render_prim == 0x0009) {
      if (ef[e2]) _swrast_Line( ctx, v2, v0 );
      if (ef[e0]) _swrast_Line( ctx, v0, v1 );
      if (ef[e1]) _swrast_Line( ctx, v1, v2 );
   } else {
      if (ef[e0]) _swrast_Line( ctx, v0, v1 );
      if (ef[e1]) _swrast_Line( ctx, v1, v2 );
      if (ef[e2]) _swrast_Line( ctx, v2, v0 );
   }

   if (ctx->_TriangleCaps & 0x1) {
      do { (v0->color)[0] = (c[0])[0]; (v0->color)[1] = (c[0])[1]; (v0->color)[2] = (c[0])[2]; (v0->color)[3] = (c[0])[3]; } while (0);
      do { (v1->color)[0] = (c[1])[0]; (v1->color)[1] = (c[1])[1]; (v1->color)[2] = (c[1])[2]; (v1->color)[3] = (c[1])[3]; } while (0);
      do { (v0->specular)[0] = (s[0])[0]; (v0->specular)[1] = (s[0])[1]; (v0->specular)[2] = (s[0])[2]; (v0->specular)[3] = (s[0])[3]; } while (0);
      do { (v1->specular)[0] = (s[1])[0]; (v1->specular)[1] = (s[1])[1]; (v1->specular)[2] = (s[1])[2]; (v1->specular)[3] = (s[1])[3]; } while (0);
      v0->index = i[0];
      v1->index = i[1];
   }
}

/* static */
 void _swsetup_render_point_tri( GLcontext *ctx,
                                      GLuint e0, GLuint e1, GLuint e2,
                                       GLuint facing )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLubyte *ef = VB->EdgeFlag;
   SWvertex *verts = swsetup->verts;
   SWvertex *v0 = &verts[e0];
   SWvertex *v1 = &verts[e1];
   SWvertex *v2 = &verts[e2];
   GLchan c[2][4];
   GLchan s[2][4];
   GLuint i[2];

   /* cull testing */
   if (ctx->Polygon.CullFlag) {
      if (facing == 1 && ctx->Polygon.CullFaceMode != 0x0404)
         return;
      if (facing == 0 && ctx->Polygon.CullFaceMode != 0x0405)
         return;
   }

   if (ctx->_TriangleCaps & 0x1) {
      do { (c[0])[0] = (v0->color)[0]; (c[0])[1] = (v0->color)[1]; (c[0])[2] = (v0->color)[2]; (c[0])[3] = (v0->color)[3]; } while (0);
      do { (c[1])[0] = (v1->color)[0]; (c[1])[1] = (v1->color)[1]; (c[1])[2] = (v1->color)[2]; (c[1])[3] = (v1->color)[3]; } while (0);
      do { (s[0])[0] = (v0->specular)[0]; (s[0])[1] = (v0->specular)[1]; (s[0])[2] = (v0->specular)[2]; (s[0])[3] = (v0->specular)[3]; } while (0);
      do { (s[1])[0] = (v1->specular)[0]; (s[1])[1] = (v1->specular)[1]; (s[1])[2] = (v1->specular)[2]; (s[1])[3] = (v1->specular)[3]; } while (0);
      i[0] = v0->index;
      i[1] = v1->index;

      do { (v0->color)[0] = (v2->color)[0]; (v0->color)[1] = (v2->color)[1]; (v0->color)[2] = (v2->color)[2]; (v0->color)[3] = (v2->color)[3]; } while (0);
      do { (v1->color)[0] = (v2->color)[0]; (v1->color)[1] = (v2->color)[1]; (v1->color)[2] = (v2->color)[2]; (v1->color)[3] = (v2->color)[3]; } while (0);
      do { (v0->specular)[0] = (v2->specular)[0]; (v0->specular)[1] = (v2->specular)[1]; (v0->specular)[2] = (v2->specular)[2]; (v0->specular)[3] = (v2->specular)[3]; } while (0);
      do { (v1->specular)[0] = (v2->specular)[0]; (v1->specular)[1] = (v2->specular)[1]; (v1->specular)[2] = (v2->specular)[2]; (v1->specular)[3] = (v2->specular)[3]; } while (0);
      v0->index = v2->index;
      v1->index = v2->index;
   }

   if (ef[e0]) _swrast_Point( ctx, v0 );
   if (ef[e1]) _swrast_Point( ctx, v1 );
   if (ef[e2]) _swrast_Point( ctx, v2 );

   if (ctx->_TriangleCaps & 0x1) {
      do { (v0->color)[0] = (c[0])[0]; (v0->color)[1] = (c[0])[1]; (v0->color)[2] = (c[0])[2]; (v0->color)[3] = (c[0])[3]; } while (0);
      do { (v1->color)[0] = (c[1])[0]; (v1->color)[1] = (c[1])[1]; (v1->color)[2] = (c[1])[2]; (v1->color)[3] = (c[1])[3]; } while (0);
      do { (v0->specular)[0] = (s[0])[0]; (v0->specular)[1] = (s[0])[1]; (v0->specular)[2] = (s[0])[2]; (v0->specular)[3] = (s[0])[3]; } while (0);
      do { (v1->specular)[0] = (s[1])[0]; (v1->specular)[1] = (s[1])[1]; (v1->specular)[2] = (s[1])[2]; (v1->specular)[3] = (s[1])[3]; } while (0);
      v0->index = i[0];
      v1->index = i[1];
   }
   _swrast_flush(ctx);
}


/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0) & 0x4) {
              if ((0) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle  return  _swrast_Triangle\n");
   }

   if ((0) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0) & 0x4) {
      if (facing == 1) {
        if ((0) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void _Optlink quadfunc( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle( ctx, v0, v1, v3 );
      triangle( ctx, v1, v2, v3 );
   }
}




static void init( void )
{
   tri_tab[(0)] = triangle;
   quad_tab[(0)] = quadfunc;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_offset(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x2) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x2) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x2) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x2) & 0x4) {
              if ((0x2) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x2) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x2) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x2) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x2) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_offset call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_offset return  _swrast_Triangle\n");
   }

   if ((0x2) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x2) & 0x4) {
      if (facing == 1) {
        if ((0x2) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void  _Optlink quadfunc_offset( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x2) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_offset( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_offset( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_offset( ctx, v0, v1, v3 );
      triangle_offset( ctx, v1, v2, v3 );
   }
}




static void init_offset( void )
{
   tri_tab[(0x2)] = triangle_offset;
   quad_tab[(0x2)] = quadfunc_offset;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_twoside(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x4) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x4) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x4) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x4) & 0x4) {
              if ((0x4) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x4) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x4) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x4) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x4) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_twoside call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_twoside return  _swrast_Triangle\n");
   }

   if ((0x4) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x4) & 0x4) {
      if (facing == 1) {
        if ((0x4) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void  _Optlink  quadfunc_twoside( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x4) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_twoside( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_twoside( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_twoside( ctx, v0, v1, v3 );
      triangle_twoside( ctx, v1, v2, v3 );
   }
}




static void init_twoside( void )
{
   tri_tab[(0x4)] = triangle_twoside;
   quad_tab[(0x4)] = quadfunc_twoside;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_offset_twoside(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x2|0x4) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x2|0x4) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if (facing == 1) {
           if ((0x2|0x4) & 0x4) {
              if ((0x2|0x4) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x2|0x4) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x2|0x4) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x2|0x4) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x2|0x4) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_offset_twoside call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_offset_twoside return  _swrast_Triangle\n");
   }

   if ((0x2|0x4) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x2|0x4) & 0x4) {
      if (facing == 1) {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void _Optlink quadfunc_offset_twoside( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
      triangle_offset_twoside( ctx, v0, v1, v3 );
      triangle_offset_twoside( ctx, v1, v2, v3 );
}




static void init_offset_twoside( void )
{
   tri_tab[(0x2|0x4)] = triangle_offset_twoside;
   quad_tab[(0x2|0x4)] = quadfunc_offset_twoside;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_unfilled(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x8) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x8) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x8) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x8) & 0x4) {
              if ((0x8) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x8) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x8) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x8) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x8) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_unfilled call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_unfilled return  _swrast_Triangle\n");
   }

   if ((0x8) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x8) & 0x4) {
      if (facing == 1) {
        if ((0x8) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void  _Optlink  quadfunc_unfilled( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x8) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_unfilled( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_unfilled( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_unfilled( ctx, v0, v1, v3 );
      triangle_unfilled( ctx, v1, v2, v3 );
   }
}




static void init_unfilled( void )
{
   tri_tab[(0x8)] = triangle_unfilled;
   quad_tab[(0x8)] = quadfunc_unfilled;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_offset_unfilled(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x2|0x8) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x2|0x8) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x2|0x8) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x2|0x8) & 0x4) {
              if ((0x2|0x8) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x2|0x8) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x2|0x8) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x2|0x8) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x2|0x8) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_offset_unfilled call _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_offset_unfilled return  _swrast_Triangle\n");
   }

   if ((0x2|0x8) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x2|0x8) & 0x4) {
      if (facing == 1) {
        if ((0x2|0x8) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void  _Optlink quadfunc_offset_unfilled( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x2|0x8) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_offset_unfilled( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_offset_unfilled( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_offset_unfilled( ctx, v0, v1, v3 );
      triangle_offset_unfilled( ctx, v1, v2, v3 );
   }
}




static void init_offset_unfilled( void )
{
   tri_tab[(0x2|0x8)] = triangle_offset_unfilled;
   quad_tab[(0x2|0x8)] = quadfunc_offset_unfilled;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_twoside_unfilled(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x4|0x8) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x4|0x8) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x4|0x8) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x4|0x8) & 0x4) {
              if ((0x4|0x8) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x4|0x8) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x4|0x8) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x4|0x8) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x4|0x8) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_twoside_unfilled call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_twoside_unfilled return  _swrast_Triangle\n");
   }

   if ((0x4|0x8) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x4|0x8) & 0x4) {
      if (facing == 1) {
        if ((0x4|0x8) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void  _Optlink  quadfunc_twoside_unfilled( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x4|0x8) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_twoside_unfilled( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_twoside_unfilled( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_twoside_unfilled( ctx, v0, v1, v3 );
      triangle_twoside_unfilled( ctx, v1, v2, v3 );
   }
}




static void init_twoside_unfilled( void )
{
   tri_tab[(0x4|0x8)] = triangle_twoside_unfilled;
   quad_tab[(0x4|0x8)] = quadfunc_twoside_unfilled;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_offset_twoside_unfilled(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x2|0x4|0x8) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x2|0x4|0x8) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x2|0x4|0x8) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x2|0x4|0x8) & 0x4) {
              if ((0x2|0x4|0x8) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x2|0x4|0x8) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x2|0x4|0x8) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x2|0x4|0x8) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x2|0x4|0x8) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_offset_twoside_unfilled call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_offset_twoside_unfilled return  _swrast_Triangle\n");
   }

   if ((0x2|0x4|0x8) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x2|0x4|0x8) & 0x4) {
      if (facing == 1) {
        if ((0x2|0x4|0x8) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void _Optlink quadfunc_offset_twoside_unfilled( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x2|0x4|0x8) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_offset_twoside_unfilled( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_offset_twoside_unfilled( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_offset_twoside_unfilled( ctx, v0, v1, v3 );
      triangle_offset_twoside_unfilled( ctx, v1, v2, v3 );
   }
}




static void init_offset_twoside_unfilled( void )
{
   tri_tab[(0x2|0x4|0x8)] = triangle_offset_twoside_unfilled;
   quad_tab[(0x2|0x4|0x8)] = quadfunc_offset_twoside_unfilled;
}

#if 0

static void triangle_rgba(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0|0x1) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0|0x1) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0|0x1) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0|0x1) & 0x4) {
              if ((0|0x1) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0|0x1) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0|0x1) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0|0x1) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0|0x1) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_rgba call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_rgba return  _swrast_Triangle\n");
   }

   if ((0|0x1) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0|0x1) & 0x4) {
      if (facing == 1) {
        if ((0|0x1) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}
#else
void triangle_rgba(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 );

#endif


#if 0

/* Need to fixup edgeflags when decomposing to triangles:
 */
/* static */

void quadfunc_rgba( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0|0x1) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_rgba( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_rgba( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      _mesa_debug(ctx, __FUNCTION__ " %i,%i,%i,%i\n",v0,v1,v2,v3);
      triangle_rgba( ctx, v0, v1, v3 );
      triangle_rgba( ctx, v1, v2, v3 );
   }
}
#else

extern void _Optlink quadfunc_rgba( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 );

#endif


static void init_rgba( void )
{
   tri_tab[(0|0x1)] = triangle_rgba;
   quad_tab[(0|0x1)] = quadfunc_rgba;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_offset_rgba(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x2|0x1) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x2|0x1) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x2|0x1) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x2|0x1) & 0x4) {
              if ((0x2|0x1) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x2|0x1) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x2|0x1) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x2|0x1) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x2|0x1) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_offset_rgba call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_offset_rgba return  _swrast_Triangle\n");
   }

   if ((0x2|0x1) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x2|0x1) & 0x4) {
      if (facing == 1) {
        if ((0x2|0x1) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void _Optlink quadfunc_offset_rgba( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x2|0x1) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_offset_rgba( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_offset_rgba( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_offset_rgba( ctx, v0, v1, v3 );
      triangle_offset_rgba( ctx, v1, v2, v3 );
   }
}




static void init_offset_rgba( void )
{
   tri_tab[(0x2|0x1)] = triangle_offset_rgba;
   quad_tab[(0x2|0x1)] = quadfunc_offset_rgba;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_twoside_rgba(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x4|0x1) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x4|0x1) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x4|0x1) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x4|0x1) & 0x4) {
              if ((0x4|0x1) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x4|0x1) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x4|0x1) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x4|0x1) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x4|0x1) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
// _mesa_debug(ctx, "triangle_twoside_rgba call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
// _mesa_debug(ctx, "triangle_twoside_rgba call  _swrast_Triangle\n");
   }

   if ((0x4|0x1) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x4|0x1) & 0x4) {
      if (facing == 1) {
        if ((0x4|0x1) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void  _Optlink  quadfunc_twoside_rgba( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x4|0x1) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_twoside_rgba( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_twoside_rgba( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_twoside_rgba( ctx, v0, v1, v3 );
      triangle_twoside_rgba( ctx, v1, v2, v3 );
   }
}




static void init_twoside_rgba( void )
{
   tri_tab[(0x4|0x1)] = triangle_twoside_rgba;
   quad_tab[(0x4|0x1)] = quadfunc_twoside_rgba;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_offset_twoside_rgba(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x2|0x4|0x1) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x2|0x4|0x1) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x2|0x4|0x1) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x2|0x4|0x1) & 0x4) {
              if ((0x2|0x4|0x1) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x2|0x4|0x1) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x2|0x4|0x1) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x2|0x4|0x1) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x2|0x4|0x1) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_offset_twoside_rgba call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_offset_twoside_rgba return  _swrast_Triangle\n");
   }

   if ((0x2|0x4|0x1) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x2|0x4|0x1) & 0x4) {
      if (facing == 1) {
        if ((0x2|0x4|0x1) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void _Optlink quadfunc_offset_twoside_rgba( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x2|0x4|0x1) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_offset_twoside_rgba( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_offset_twoside_rgba( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_offset_twoside_rgba( ctx, v0, v1, v3 );
      triangle_offset_twoside_rgba( ctx, v1, v2, v3 );
   }
}




static void init_offset_twoside_rgba( void )
{
   tri_tab[(0x2|0x4|0x1)] = triangle_offset_twoside_rgba;
   quad_tab[(0x2|0x4|0x1)] = quadfunc_offset_twoside_rgba;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_unfilled_rgba(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x8|0x1) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x8|0x1) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x8|0x1) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x8|0x1) & 0x4) {
              if ((0x8|0x1) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x8|0x1) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x8|0x1) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x8|0x1) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x8|0x1) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_unfilled_rgba call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_unfilled_rgba return  _swrast_Triangle\n");
   }

   if ((0x8|0x1) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x8|0x1) & 0x4) {
      if (facing == 1) {
        if ((0x8|0x1) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void  _Optlink  quadfunc_unfilled_rgba( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x8|0x1) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_unfilled_rgba( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_unfilled_rgba( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_unfilled_rgba( ctx, v0, v1, v3 );
      triangle_unfilled_rgba( ctx, v1, v2, v3 );
   }
}




static void init_unfilled_rgba( void )
{
   tri_tab[(0x8|0x1)] = triangle_unfilled_rgba;
   quad_tab[(0x8|0x1)] = quadfunc_unfilled_rgba;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_offset_unfilled_rgba(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x2|0x8|0x1) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x2|0x8|0x1) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x2|0x8|0x1) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x2|0x8|0x1) & 0x4) {
              if ((0x2|0x8|0x1) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x2|0x8|0x1) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x2|0x8|0x1) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x2|0x8|0x1) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x2|0x8|0x1) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_offset_unfilled_rgba call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_offset_unfilled_rgba return  _swrast_Triangle\n");
   }

   if ((0x2|0x8|0x1) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x2|0x8|0x1) & 0x4) {
      if (facing == 1) {
        if ((0x2|0x8|0x1) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void  _Optlink  quadfunc_offset_unfilled_rgba( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x2|0x8|0x1) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_offset_unfilled_rgba( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_offset_unfilled_rgba( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_offset_unfilled_rgba( ctx, v0, v1, v3 );
      triangle_offset_unfilled_rgba( ctx, v1, v2, v3 );
   }
}




static void init_offset_unfilled_rgba( void )
{
   tri_tab[(0x2|0x8|0x1)] = triangle_offset_unfilled_rgba;
   quad_tab[(0x2|0x8|0x1)] = quadfunc_offset_unfilled_rgba;
}



static void triangle_twoside_unfilled_rgba(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x4|0x8|0x1) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x4|0x8|0x1) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x4|0x8|0x1) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x4|0x8|0x1) & 0x4) {
              if ((0x4|0x8|0x1) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

   }

    v[0]->win[2] += offset;
    v[1]->win[2] += offset;
    v[2]->win[2] += offset;
// _mesa_debug(ctx, "triangle_twoside_unfilled_rgba call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
// _mesa_debug(ctx, "triangle_twoside_unfilled_rgba return  _swrast_Triangle\n");

   if ((0x4|0x8|0x1) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x4|0x8|0x1) & 0x4) {
      if (facing == 1) {
        if ((0x4|0x8|0x1) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void  _Optlink  quadfunc_twoside_unfilled_rgba( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x4|0x8|0x1) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_twoside_unfilled_rgba( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_twoside_unfilled_rgba( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_twoside_unfilled_rgba( ctx, v0, v1, v3 );
      triangle_twoside_unfilled_rgba( ctx, v1, v2, v3 );
   }
}




static void init_twoside_unfilled_rgba( void )
{
   tri_tab[(0x4|0x8|0x1)] = triangle_twoside_unfilled_rgba;
   quad_tab[(0x4|0x8|0x1)] = quadfunc_twoside_unfilled_rgba;
}



/* $Id: ss_tritmp.h,v 1.20 2002/11/13 15:04:29 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  5.0
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
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */


static void triangle_offset_twoside_unfilled_rgba(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   SWvertex *v[3];
   GLfloat z[3];
   GLfloat offset;
   GLenum mode = 0x1B02;
   GLuint facing = 0;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];


   if ((0x2|0x4|0x8|0x1) & (0x4 | 0x2 | 0x8))
   {
      GLfloat ex = v[0]->win[0] - v[2]->win[0];
      GLfloat ey = v[0]->win[1] - v[2]->win[1];
      GLfloat fx = v[1]->win[0] - v[2]->win[0];
      GLfloat fy = v[1]->win[1] - v[2]->win[1];
      GLfloat cc  = ex*fy - ey*fx;

      if ((0x2|0x4|0x8|0x1) & (0x4 | 0x8))
      {
        facing = (cc < 0.0) ^ ctx->Polygon._FrontBit;
         if (ctx->Stencil.TestTwoSide)
            ctx->_Facing = facing; /* for 2-sided stencil test */

        if ((0x2|0x4|0x8|0x1) & 0x8)
           mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;

        if (facing == 1) {
           if ((0x2|0x4|0x8|0x1) & 0x4) {
              if ((0x2|0x4|0x8|0x1) & 0x1) {
                 GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[1]->Ptr;
                 do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
                 do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
                 do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
                 if (VB->SecondaryColorPtr[1]) {
                    GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[1]->Ptr;
                    do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
                    do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
                    do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
                 }
              } else {
                 GLuint *vbindex = VB->IndexPtr[1]->data;
                 (v[0]->index = vbindex[e0]);
                 (v[1]->index = vbindex[e1]);
                 (v[2]->index = vbindex[e2]);
              }
           }
        }
      }

      if ((0x2|0x4|0x8|0x1) & 0x2)
      {
        offset = ctx->Polygon.OffsetUnits;
        z[0] = v[0]->win[2];
        z[1] = v[1]->win[2];
        z[2] = v[2]->win[2];
        if (cc * cc > 1e-16) {
           GLfloat ez = z[0] - z[2];
           GLfloat fz = z[1] - z[2];
           GLfloat a = ey*fz - ez*fy;
           GLfloat b = ez*fx - ex*fz;
           GLfloat ic = 1.0F / cc;
           GLfloat ac = a * ic;
           GLfloat bc = b * ic;
           if (ac < 0.0F) ac = -ac;
           if (bc < 0.0F) bc = -bc;
           offset += ( (ac)>(bc) ? (ac) : (bc) ) * ctx->Polygon.OffsetFactor;
        }
         offset *= ctx->MRD;
         /*printf("offset %g\n", offset);*/
      }
   }

   if (mode == 0x1B00) {
      if (((0x2|0x4|0x8|0x1) & 0x2) && ctx->Polygon.OffsetPoint) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_point_tri( ctx, e0, e1, e2, facing );
   } else if (mode == 0x1B01) {
      if (((0x2|0x4|0x8|0x1) & 0x2) && ctx->Polygon.OffsetLine) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
      _swsetup_render_line_tri( ctx, e0, e1, e2, facing );
   } else {
      if (((0x2|0x4|0x8|0x1) & 0x2) && ctx->Polygon.OffsetFill) {
        v[0]->win[2] += offset;
        v[1]->win[2] += offset;
        v[2]->win[2] += offset;
      }
 _mesa_debug(ctx, "triangle_offset_twoside_unfilled_rgba call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
 _mesa_debug(ctx, "triangle_offset_twoside_unfilled_rgba return  _swrast_Triangle\n");
   }

   if ((0x2|0x4|0x8|0x1) & 0x2) {
      v[0]->win[2] = z[0];
      v[1]->win[2] = z[1];
      v[2]->win[2] = z[2];
   }

   if ((0x2|0x4|0x8|0x1) & 0x4) {
      if (facing == 1) {
        if ((0x2|0x4|0x8|0x1) & 0x1) {
           GLchan (*vbcolor)[4] = (GLchan (*)[4])VB->ColorPtr[0]->Ptr;
           do { (v[0]->color)[0] = (vbcolor[e0])[0]; (v[0]->color)[1] = (vbcolor[e0])[1]; (v[0]->color)[2] = (vbcolor[e0])[2]; (v[0]->color)[3] = (vbcolor[e0])[3]; } while (0);
           do { (v[1]->color)[0] = (vbcolor[e1])[0]; (v[1]->color)[1] = (vbcolor[e1])[1]; (v[1]->color)[2] = (vbcolor[e1])[2]; (v[1]->color)[3] = (vbcolor[e1])[3]; } while (0);
           do { (v[2]->color)[0] = (vbcolor[e2])[0]; (v[2]->color)[1] = (vbcolor[e2])[1]; (v[2]->color)[2] = (vbcolor[e2])[2]; (v[2]->color)[3] = (vbcolor[e2])[3]; } while (0);
           if (VB->SecondaryColorPtr[0]) {
              GLchan (*vbspec)[4] = (GLchan (*)[4])VB->SecondaryColorPtr[0]->Ptr;
              do { (v[0]->specular)[0] = (vbspec[e0])[0]; (v[0]->specular)[1] = (vbspec[e0])[1]; (v[0]->specular)[2] = (vbspec[e0])[2]; } while (0);
              do { (v[1]->specular)[0] = (vbspec[e1])[0]; (v[1]->specular)[1] = (vbspec[e1])[1]; (v[1]->specular)[2] = (vbspec[e1])[2]; } while (0);
              do { (v[2]->specular)[0] = (vbspec[e2])[0]; (v[2]->specular)[1] = (vbspec[e2])[1]; (v[2]->specular)[2] = (vbspec[e2])[2]; } while (0);
           }
        } else {
           GLuint *vbindex = VB->IndexPtr[0]->data;
           (v[0]->index = vbindex[e0]);
           (v[1]->index = vbindex[e1]);
           (v[2]->index = vbindex[e2]);
        }
      }
   }
}



/* Need to fixup edgeflags when decomposing to triangles:
 */
static void _Optlink quadfunc_offset_twoside_unfilled_rgba( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{
   if ((0x2|0x4|0x8|0x1) & 0x8) {
      struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
      GLubyte ef1 = VB->EdgeFlag[v1];
      GLubyte ef3 = VB->EdgeFlag[v3];
      VB->EdgeFlag[v1] = 0;
      triangle_offset_twoside_unfilled_rgba( ctx, v0, v1, v3 );
      VB->EdgeFlag[v1] = ef1;
      VB->EdgeFlag[v3] = 0;
      triangle_offset_twoside_unfilled_rgba( ctx, v1, v2, v3 );
      VB->EdgeFlag[v3] = ef3;
   } else {
      triangle_offset_twoside_unfilled_rgba( ctx, v0, v1, v3 );
      triangle_offset_twoside_unfilled_rgba( ctx, v1, v2, v3 );
   }
}




static void init_offset_twoside_unfilled_rgba( void )
{
   tri_tab[(0x2|0x4|0x8|0x1)] = triangle_offset_twoside_unfilled_rgba;
   quad_tab[(0x2|0x4|0x8|0x1)] = quadfunc_offset_twoside_unfilled_rgba;
}




void _swsetup_trifuncs_init( GLcontext *ctx )
{
   (void) ctx;

   init();
   init_offset();
   init_twoside();
   init_offset_twoside();
   init_unfilled();
   init_offset_unfilled();
   init_twoside_unfilled();
   init_offset_twoside_unfilled();

   init_rgba();
   init_offset_rgba();
   init_twoside_rgba();
   init_offset_twoside_rgba();
   init_unfilled_rgba();
   init_offset_unfilled_rgba();
   init_twoside_unfilled_rgba();
   init_offset_twoside_unfilled_rgba();
}


static void swsetup_points( GLcontext *ctx, GLuint first, GLuint last )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   GLuint i;

   if (VB->Elts) {
      for (i = first; i < last; i++)
        if (VB->ClipMask[VB->Elts[i]] == 0)
           _swrast_Point( ctx, &verts[VB->Elts[i]] );
   }
   else {
      for (i = first; i < last; i++)
        if (VB->ClipMask[i] == 0)
           _swrast_Point( ctx, &verts[i] );
   }
}

static void swsetup_line( GLcontext *ctx, GLuint v0, GLuint v1 )
{
   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
   _swrast_Line( ctx, &verts[v0], &verts[v1] );
}



void _swsetup_choose_trifuncs( GLcontext *ctx )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   GLuint ind = 0;

   if (ctx->Polygon.OffsetPoint ||
       ctx->Polygon.OffsetLine ||
       ctx->Polygon.OffsetFill)
      ind |= 0x2;

   if (ctx->Light.Enabled && ctx->Light.Model.TwoSide)
      ind |= 0x4;

   /* We piggyback the two-sided stencil front/back determination on the
    * unfilled triangle path.
    */
   if ((ctx->_TriangleCaps & 0x10) ||
       (ctx->Stencil.Enabled && ctx->Stencil.TestTwoSide))
      ind |= 0x8;

   if (ctx->Visual.rgbMode)
      ind |= 0x1;

   tnl->Driver.Render.Triangle = tri_tab[ind];
   tnl->Driver.Render.Quad = quad_tab[ind];
   tnl->Driver.Render.Line = swsetup_line;
   tnl->Driver.Render.Points = swsetup_points;

   ctx->_Facing = 0;
}
