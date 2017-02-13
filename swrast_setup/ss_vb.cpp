/* $Id: ss_vb.c,v 1.22 2002/10/29 20:29:00 brianp Exp $ */
/* Expanded version by EK */

#include "glheader.h"
#include "colormac.h"
#include "context.h"
#include "macros.h"
#include "imports.h"

#include "swrast/swrast.h"
#include "tnl/t_context.h"
#include "math/m_vector.h"
#include "math/m_translate.h"

#include "ss_context.h"
#include "ss_vb.h"



/*
 * Enable/disable features (blocks of code) by setting FEATURE_xyz to 0 or 1.
 */


static void do_import( struct vertex_buffer *VB,
                      struct gl_client_array *to,
                      struct gl_client_array *from )
{
   GLuint count = VB->Count;

   if (!to->Ptr) {
      to->Ptr = _mesa_align_malloc(VB->Size * 4 * sizeof(GLchan), 32);
      to->Type = CHAN_TYPE;
   }

   /* No need to transform the same value 3000 times.
    */
   if (!from->StrideB) {
      to->StrideB = 0;
      count = 1;
   }
   else
      to->StrideB = 4 * sizeof(GLchan);

   _math_trans_4chan( (GLchan (*)[4]) to->Ptr,
                     from->Ptr,
                     from->StrideB,
                     from->Type,
                     from->Size,
                     0,
                     count);
}

static void import_float_colors( GLcontext *ctx )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   struct gl_client_array *to = &((SScontext *)ctx->swsetup_context)->ChanColor;
   do_import( VB, to, VB->ColorPtr[0] );
   VB->ColorPtr[0] = to;
}

static void import_float_spec_colors( GLcontext *ctx )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   struct gl_client_array *to = &((SScontext *)ctx->swsetup_context)->ChanSecondaryColor;
   do_import( VB, to, VB->SecondaryColorPtr[0] );
   VB->SecondaryColorPtr[0] = to;
}


/* Provides a RasterSetup function which prebuilds vertices for the
 * software rasterizer.  This is required for the triangle functions
 * in this module, but not the rest of the swrast module.
 */

#define COLOR         0x1
#define INDEX         0x2
#define TEX0          0x4
#define MULTITEX      0x8
#define SPEC          0x10
#define FOG           0x20
#define POINT         0x40
#define MAX_SETUPFUNC 0x80


static setup_func setup_tab[MAX_SETUPFUNC];
static interp_func interp_tab[MAX_SETUPFUNC];
static copy_pv_func copy_pv_tab[MAX_SETUPFUNC];


static void emit_none(GLcontext *ctx, GLuint start, GLuint end,
                     GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;              /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;

   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
        v->win[0] = sx * proj[0] + tx;
        v->win[1] = sy * proj[1] + ty;
        v->win[2] = sz * proj[2] + tz;
        v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));
   }
}



static void interp_none( GLcontext *ctx,
                        GLfloat t,
                        GLuint edst, GLuint eout, GLuint ein,
                        GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }
}


static void copy_pv_none( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];
}


static void init_none( void )
{
   setup_tab[(0)] = emit_none;
   interp_tab[(0)] = interp_none;
   copy_pv_tab[(0)] = copy_pv_none;
}


static void emit_color(GLcontext *ctx, GLuint start, GLuint end,
                     GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;              /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      if (VB->ColorPtr[0]->Type != 0x1401)
        import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
        v->win[0] = sx * proj[0] + tx;
        v->win[1] = sy * proj[1] + ty;
        v->win[2] = sz * proj[2] + tz;
        v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));

       (v->color)[0] = (color)[0];
       (v->color)[1] = (color)[1];
       (v->color)[2] = (color)[2];
       (v->color)[3] = (color)[3];
       (color = (GLchan *)((GLubyte *)color + color_stride));
   }
}


static void interp_color( GLcontext *ctx,
                        GLfloat t,
                        GLuint edst, GLuint eout, GLuint ein,
                        GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };


}


static void copy_pv_color( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

   (dst->color)[0] = (src->color)[0];
   (dst->color)[1] = (src->color)[1];
   (dst->color)[2] = (src->color)[2];
   (dst->color)[3] = (src->color)[3];
}


static void init_color( void )
{
   setup_tab[(0x1)] = emit_color;
   interp_tab[(0x1)] = interp_color;
   copy_pv_tab[(0x1)] = copy_pv_color;
}


static void emit_color_spec(GLcontext *ctx, GLuint start, GLuint end,
                     GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;              /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      if (VB->ColorPtr[0]->Type != 0x1401)
        import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;

      if (VB->SecondaryColorPtr[0]->Type != 0x1401)
        import_float_spec_colors( ctx );

      spec = (GLchan *) VB->SecondaryColorPtr[0]->Ptr;
      spec_stride = VB->SecondaryColorPtr[0]->StrideB;


   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
        v->win[0] = sx * proj[0] + tx;
        v->win[1] = sy * proj[1] + ty;
        v->win[2] = sz * proj[2] + tz;
        v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));


        { (v->color)[0] = (color)[0];
          (v->color)[1] = (color)[1];
          (v->color)[2] = (color)[2];
          (v->color)[3] = (color)[3];
        }
        (color = (GLchan *)((GLubyte *)color + color_stride));

        {
          (v->specular)[0] = (spec)[0];
          (v->specular)[1] = (spec)[1];
          (v->specular)[2] = (spec)[2];
          (v->specular)[3] = (spec)[3];
        }
        (spec = (GLchan *)((GLubyte *)spec + spec_stride));
   }
}


static void interp_color_spec( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }



      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };



}


static void copy_pv_color_spec( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };

      { (dst->specular)[0] = (src->specular)[0]; (dst->specular)[1] = (src->specular)[1]; (dst->specular)[2] = (src->specular)[2]; };

}


static void init_color_spec( void )
{
   setup_tab[(0x1|0x10)] = emit_color_spec;
   interp_tab[(0x1|0x10)] = interp_color_spec;
   copy_pv_tab[(0x1|0x10)] = copy_pv_color_spec;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_fog(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));



    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));


    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));


   }
}



static void interp_color_fog( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }



      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };


      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));


}


static void copy_pv_color_fog( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };


}


static void init_color_fog( void )
{
   setup_tab[(0x1|0x20)] = emit_color_fog;
   interp_tab[(0x1|0x20)] = interp_color_fog;
   copy_pv_tab[(0x1|0x20)] = copy_pv_color_fog;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_spec_fog(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      if (VB->SecondaryColorPtr[0]->Type != 0x1401)
    import_float_spec_colors( ctx );

      spec = (GLchan *) VB->SecondaryColorPtr[0]->Ptr;
      spec_stride = VB->SecondaryColorPtr[0]->StrideB;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));



    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));

    { (v->specular)[0] = (spec)[0]; (v->specular)[1] = (spec)[1]; (v->specular)[2] = (spec)[2]; (v->specular)[3] = (spec)[3]; };
    (spec = (GLchan *)((GLubyte *)spec + spec_stride));

    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));


   }
}



static void interp_color_spec_fog( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }



      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));


}


static void copy_pv_color_spec_fog( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };

      { (dst->specular)[0] = (src->specular)[0]; (dst->specular)[1] = (src->specular)[1]; (dst->specular)[2] = (src->specular)[2]; };

}


static void init_color_spec_fog( void )
{
   setup_tab[(0x1|0x10|0x20)] = emit_color_spec_fog;
   interp_tab[(0x1|0x10|0x20)] = interp_color_spec_fog;
   copy_pv_tab[(0x1|0x10|0x20)] = copy_pv_color_spec_fog;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_tex0(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;

//   if (IND & TEX0) {
      tc[0] = (GLfloat *)VB->TexCoordPtr[0]->data;
      tsz[0] = VB->TexCoordPtr[0]->size;
      tstride[0] = VB->TexCoordPtr[0]->stride;
//   }

   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));

    { { v->texcoord[0][0] = 0; v->texcoord[0][1] = 0; v->texcoord[0][2] = 0; v->texcoord[0][3] = 1; }; { switch (tsz[0]) { case 4: (v->texcoord[0])[3] = (tc[0])[3]; case 3: (v->texcoord[0])[2] = (tc[0])[2]; case 2: (v->texcoord[0])[1] = (tc[0])[1]; case 1: (v->texcoord[0])[0] = (tc[0])[0]; } }; };
    (tc[0] = (GLfloat *)((GLubyte *)tc[0] + tstride[0]));


    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));




   }
}



static void interp_color_tex0( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }

      { dst->texcoord[0][0] = (((out->texcoord[0])[0]) + ((t)) * (((in->texcoord[0])[0]) - ((out->texcoord[0])[0]))); dst->texcoord[0][1] = (((out->texcoord[0])[1]) + ((t)) * (((in->texcoord[0])[1]) - ((out->texcoord[0])[1]))); dst->texcoord[0][2] = (((out->texcoord[0])[2]) + ((t)) * (((in->texcoord[0])[2]) - ((out->texcoord[0])[2]))); dst->texcoord[0][3] = (((out->texcoord[0])[3]) + ((t)) * (((in->texcoord[0])[3]) - ((out->texcoord[0])[3]))); };


      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };




}


static void copy_pv_color_tex0( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };


}


static void init_color_tex0( void )
{
   setup_tab[(0x1|0x4)] = emit_color_tex0;
   interp_tab[(0x1|0x4)] = interp_color_tex0;
   copy_pv_tab[(0x1|0x4)] = copy_pv_color_tex0;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_tex0_spec(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;

//   if (IND & TEX0) {
      tc[0] = (GLfloat *)VB->TexCoordPtr[0]->data;
      tsz[0] = VB->TexCoordPtr[0]->size;
      tstride[0] = VB->TexCoordPtr[0]->stride;
//   }

   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      if (VB->SecondaryColorPtr[0]->Type != 0x1401)
    import_float_spec_colors( ctx );

      spec = (GLchan *) VB->SecondaryColorPtr[0]->Ptr;
      spec_stride = VB->SecondaryColorPtr[0]->StrideB;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));

    { { v->texcoord[0][0] = 0; v->texcoord[0][1] = 0; v->texcoord[0][2] = 0; v->texcoord[0][3] = 1; }; { switch (tsz[0]) { case 4: (v->texcoord[0])[3] = (tc[0])[3]; case 3: (v->texcoord[0])[2] = (tc[0])[2]; case 2: (v->texcoord[0])[1] = (tc[0])[1]; case 1: (v->texcoord[0])[0] = (tc[0])[0]; } }; };
    (tc[0] = (GLfloat *)((GLubyte *)tc[0] + tstride[0]));


    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));

    { (v->specular)[0] = (spec)[0]; (v->specular)[1] = (spec)[1]; (v->specular)[2] = (spec)[2]; (v->specular)[3] = (spec)[3]; };
    (spec = (GLchan *)((GLubyte *)spec + spec_stride));



   }
}



static void interp_color_tex0_spec( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }

      { dst->texcoord[0][0] = (((out->texcoord[0])[0]) + ((t)) * (((in->texcoord[0])[0]) - ((out->texcoord[0])[0]))); dst->texcoord[0][1] = (((out->texcoord[0])[1]) + ((t)) * (((in->texcoord[0])[1]) - ((out->texcoord[0])[1]))); dst->texcoord[0][2] = (((out->texcoord[0])[2]) + ((t)) * (((in->texcoord[0])[2]) - ((out->texcoord[0])[2]))); dst->texcoord[0][3] = (((out->texcoord[0])[3]) + ((t)) * (((in->texcoord[0])[3]) - ((out->texcoord[0])[3]))); };


      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };



}


static void copy_pv_color_tex0_spec( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };

      { (dst->specular)[0] = (src->specular)[0]; (dst->specular)[1] = (src->specular)[1]; (dst->specular)[2] = (src->specular)[2]; };

}


static void init_color_tex0_spec( void )
{
   setup_tab[(0x1|0x4|0x10)] = emit_color_tex0_spec;
   interp_tab[(0x1|0x4|0x10)] = interp_color_tex0_spec;
   copy_pv_tab[(0x1|0x4|0x10)] = copy_pv_color_tex0_spec;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_tex0_fog(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;

//   if (IND & TEX0) {
      tc[0] = (GLfloat *)VB->TexCoordPtr[0]->data;
      tsz[0] = VB->TexCoordPtr[0]->size;
      tstride[0] = VB->TexCoordPtr[0]->stride;
//   }

   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));

    { { v->texcoord[0][0] = 0; v->texcoord[0][1] = 0; v->texcoord[0][2] = 0; v->texcoord[0][3] = 1; }; { switch (tsz[0]) { case 4: (v->texcoord[0])[3] = (tc[0])[3]; case 3: (v->texcoord[0])[2] = (tc[0])[2]; case 2: (v->texcoord[0])[1] = (tc[0])[1]; case 1: (v->texcoord[0])[0] = (tc[0])[0]; } }; };
    (tc[0] = (GLfloat *)((GLubyte *)tc[0] + tstride[0]));


    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));


    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));


   }
}



static void interp_color_tex0_fog( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }

      { dst->texcoord[0][0] = (((out->texcoord[0])[0]) + ((t)) * (((in->texcoord[0])[0]) - ((out->texcoord[0])[0]))); dst->texcoord[0][1] = (((out->texcoord[0])[1]) + ((t)) * (((in->texcoord[0])[1]) - ((out->texcoord[0])[1]))); dst->texcoord[0][2] = (((out->texcoord[0])[2]) + ((t)) * (((in->texcoord[0])[2]) - ((out->texcoord[0])[2]))); dst->texcoord[0][3] = (((out->texcoord[0])[3]) + ((t)) * (((in->texcoord[0])[3]) - ((out->texcoord[0])[3]))); };


      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };


      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));


}


static void copy_pv_color_tex0_fog( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };


}


static void init_color_tex0_fog( void )
{
   setup_tab[(0x1|0x4|0x20)] = emit_color_tex0_fog;
   interp_tab[(0x1|0x4|0x20)] = interp_color_tex0_fog;
   copy_pv_tab[(0x1|0x4|0x20)] = copy_pv_color_tex0_fog;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_tex0_spec_fog(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;

//   if (IND & TEX0) {
      tc[0] = (GLfloat *)VB->TexCoordPtr[0]->data;
      tsz[0] = VB->TexCoordPtr[0]->size;
      tstride[0] = VB->TexCoordPtr[0]->stride;
//   }

   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      if (VB->SecondaryColorPtr[0]->Type != 0x1401)
    import_float_spec_colors( ctx );

      spec = (GLchan *) VB->SecondaryColorPtr[0]->Ptr;
      spec_stride = VB->SecondaryColorPtr[0]->StrideB;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));

    { { v->texcoord[0][0] = 0; v->texcoord[0][1] = 0; v->texcoord[0][2] = 0; v->texcoord[0][3] = 1; }; { switch (tsz[0]) { case 4: (v->texcoord[0])[3] = (tc[0])[3]; case 3: (v->texcoord[0])[2] = (tc[0])[2]; case 2: (v->texcoord[0])[1] = (tc[0])[1]; case 1: (v->texcoord[0])[0] = (tc[0])[0]; } }; };
    (tc[0] = (GLfloat *)((GLubyte *)tc[0] + tstride[0]));


    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));

    { (v->specular)[0] = (spec)[0]; (v->specular)[1] = (spec)[1]; (v->specular)[2] = (spec)[2]; (v->specular)[3] = (spec)[3]; };
    (spec = (GLchan *)((GLubyte *)spec + spec_stride));

    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));


   }
}



static void interp_color_tex0_spec_fog( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }

      { dst->texcoord[0][0] = (((out->texcoord[0])[0]) + ((t)) * (((in->texcoord[0])[0]) - ((out->texcoord[0])[0]))); dst->texcoord[0][1] = (((out->texcoord[0])[1]) + ((t)) * (((in->texcoord[0])[1]) - ((out->texcoord[0])[1]))); dst->texcoord[0][2] = (((out->texcoord[0])[2]) + ((t)) * (((in->texcoord[0])[2]) - ((out->texcoord[0])[2]))); dst->texcoord[0][3] = (((out->texcoord[0])[3]) + ((t)) * (((in->texcoord[0])[3]) - ((out->texcoord[0])[3]))); };


      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));


}


static void copy_pv_color_tex0_spec_fog( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };

      { (dst->specular)[0] = (src->specular)[0]; (dst->specular)[1] = (src->specular)[1]; (dst->specular)[2] = (src->specular)[2]; };

}


static void init_color_tex0_spec_fog( void )
{
   setup_tab[(0x1|0x4|0x10|0x20)] = emit_color_tex0_spec_fog;
   interp_tab[(0x1|0x4|0x10|0x20)] = interp_color_tex0_spec_fog;
   copy_pv_tab[(0x1|0x4|0x10|0x20)] = copy_pv_color_tex0_spec_fog;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_multitex(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


      for (i = 0 ; i < ctx->Const.MaxTextureUnits ; i++) {
    if (VB->TexCoordPtr[i]) {
       maxtex = i+1;
       tc[i] = (GLfloat *)VB->TexCoordPtr[i]->data;
       tsz[i] = VB->TexCoordPtr[i]->size;
       tstride[i] = VB->TexCoordPtr[i]->stride;
    }
    else tc[i] = 0;
      }
   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));


    GLuint u;
    for (u = 0 ; u < maxtex ; u++)
       if (tc[u]) {
          { { v->texcoord[u][0] = 0; v->texcoord[u][1] = 0; v->texcoord[u][2] = 0; v->texcoord[u][3] = 1; }; { switch (tsz[u]) { case 4: (v->texcoord[u])[3] = (tc[u])[3]; case 3: (v->texcoord[u])[2] = (tc[u])[2]; case 2: (v->texcoord[u])[1] = (tc[u])[1]; case 1: (v->texcoord[u])[0] = (tc[u])[0]; } }; };
          (tc[u] = (GLfloat *)((GLubyte *)tc[u] + tstride[u]));
       }

    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));




   }
}



static void interp_color_multitex( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }


      GLuint u;
      GLuint maxtex = ctx->Const.MaxTextureUnits;
      for (u = 0 ; u < maxtex ; u++)
    if (VB->TexCoordPtr[u]) {
       { dst->texcoord[u][0] = (((out->texcoord[u])[0]) + ((t)) * (((in->texcoord[u])[0]) - ((out->texcoord[u])[0]))); dst->texcoord[u][1] = (((out->texcoord[u])[1]) + ((t)) * (((in->texcoord[u])[1]) - ((out->texcoord[u])[1]))); dst->texcoord[u][2] = (((out->texcoord[u])[2]) + ((t)) * (((in->texcoord[u])[2]) - ((out->texcoord[u])[2]))); dst->texcoord[u][3] = (((out->texcoord[u])[3]) + ((t)) * (((in->texcoord[u])[3]) - ((out->texcoord[u])[3]))); };
    }

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };




}


static void copy_pv_color_multitex( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };


}


static void init_color_multitex( void )
{
   setup_tab[(0x1|0x8)] = emit_color_multitex;
   interp_tab[(0x1|0x8)] = interp_color_multitex;
   copy_pv_tab[(0x1|0x8)] = copy_pv_color_multitex;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_multitex_spec(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


      for (i = 0 ; i < ctx->Const.MaxTextureUnits ; i++) {
    if (VB->TexCoordPtr[i]) {
       maxtex = i+1;
       tc[i] = (GLfloat *)VB->TexCoordPtr[i]->data;
       tsz[i] = VB->TexCoordPtr[i]->size;
       tstride[i] = VB->TexCoordPtr[i]->stride;
    }
    else tc[i] = 0;
      }
   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      if (VB->SecondaryColorPtr[0]->Type != 0x1401)
    import_float_spec_colors( ctx );

      spec = (GLchan *) VB->SecondaryColorPtr[0]->Ptr;
      spec_stride = VB->SecondaryColorPtr[0]->StrideB;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));


    GLuint u;
    for (u = 0 ; u < maxtex ; u++)
       if (tc[u]) {
          { { v->texcoord[u][0] = 0; v->texcoord[u][1] = 0; v->texcoord[u][2] = 0; v->texcoord[u][3] = 1; }; { switch (tsz[u]) { case 4: (v->texcoord[u])[3] = (tc[u])[3]; case 3: (v->texcoord[u])[2] = (tc[u])[2]; case 2: (v->texcoord[u])[1] = (tc[u])[1]; case 1: (v->texcoord[u])[0] = (tc[u])[0]; } }; };
          (tc[u] = (GLfloat *)((GLubyte *)tc[u] + tstride[u]));
       }

    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));

    { (v->specular)[0] = (spec)[0]; (v->specular)[1] = (spec)[1]; (v->specular)[2] = (spec)[2]; (v->specular)[3] = (spec)[3]; };
    (spec = (GLchan *)((GLubyte *)spec + spec_stride));



   }
}



static void interp_color_multitex_spec( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }


      GLuint u;
      GLuint maxtex = ctx->Const.MaxTextureUnits;
      for (u = 0 ; u < maxtex ; u++)
    if (VB->TexCoordPtr[u]) {
       { dst->texcoord[u][0] = (((out->texcoord[u])[0]) + ((t)) * (((in->texcoord[u])[0]) - ((out->texcoord[u])[0]))); dst->texcoord[u][1] = (((out->texcoord[u])[1]) + ((t)) * (((in->texcoord[u])[1]) - ((out->texcoord[u])[1]))); dst->texcoord[u][2] = (((out->texcoord[u])[2]) + ((t)) * (((in->texcoord[u])[2]) - ((out->texcoord[u])[2]))); dst->texcoord[u][3] = (((out->texcoord[u])[3]) + ((t)) * (((in->texcoord[u])[3]) - ((out->texcoord[u])[3]))); };
    }

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };



}


static void copy_pv_color_multitex_spec( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };

      { (dst->specular)[0] = (src->specular)[0]; (dst->specular)[1] = (src->specular)[1]; (dst->specular)[2] = (src->specular)[2]; };

}


static void init_color_multitex_spec( void )
{
   setup_tab[(0x1|0x8|0x10)] = emit_color_multitex_spec;
   interp_tab[(0x1|0x8|0x10)] = interp_color_multitex_spec;
   copy_pv_tab[(0x1|0x8|0x10)] = copy_pv_color_multitex_spec;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_multitex_fog(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


      for (i = 0 ; i < ctx->Const.MaxTextureUnits ; i++) {
    if (VB->TexCoordPtr[i]) {
       maxtex = i+1;
       tc[i] = (GLfloat *)VB->TexCoordPtr[i]->data;
       tsz[i] = VB->TexCoordPtr[i]->size;
       tstride[i] = VB->TexCoordPtr[i]->stride;
    }
    else tc[i] = 0;
      }
   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));


    GLuint u;
    for (u = 0 ; u < maxtex ; u++)
       if (tc[u]) {
          { { v->texcoord[u][0] = 0; v->texcoord[u][1] = 0; v->texcoord[u][2] = 0; v->texcoord[u][3] = 1; }; { switch (tsz[u]) { case 4: (v->texcoord[u])[3] = (tc[u])[3]; case 3: (v->texcoord[u])[2] = (tc[u])[2]; case 2: (v->texcoord[u])[1] = (tc[u])[1]; case 1: (v->texcoord[u])[0] = (tc[u])[0]; } }; };
          (tc[u] = (GLfloat *)((GLubyte *)tc[u] + tstride[u]));
       }

    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));


    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));


   }
}



static void interp_color_multitex_fog( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }


      GLuint u;
      GLuint maxtex = ctx->Const.MaxTextureUnits;
      for (u = 0 ; u < maxtex ; u++)
    if (VB->TexCoordPtr[u]) {
       { dst->texcoord[u][0] = (((out->texcoord[u])[0]) + ((t)) * (((in->texcoord[u])[0]) - ((out->texcoord[u])[0]))); dst->texcoord[u][1] = (((out->texcoord[u])[1]) + ((t)) * (((in->texcoord[u])[1]) - ((out->texcoord[u])[1]))); dst->texcoord[u][2] = (((out->texcoord[u])[2]) + ((t)) * (((in->texcoord[u])[2]) - ((out->texcoord[u])[2]))); dst->texcoord[u][3] = (((out->texcoord[u])[3]) + ((t)) * (((in->texcoord[u])[3]) - ((out->texcoord[u])[3]))); };
    }

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };


      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));


}


static void copy_pv_color_multitex_fog( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };


}


static void init_color_multitex_fog( void )
{
   setup_tab[(0x1|0x8|0x20)] = emit_color_multitex_fog;
   interp_tab[(0x1|0x8|0x20)] = interp_color_multitex_fog;
   copy_pv_tab[(0x1|0x8|0x20)] = copy_pv_color_multitex_fog;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_multitex_spec_fog(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


      for (i = 0 ; i < ctx->Const.MaxTextureUnits ; i++) {
    if (VB->TexCoordPtr[i]) {
       maxtex = i+1;
       tc[i] = (GLfloat *)VB->TexCoordPtr[i]->data;
       tsz[i] = VB->TexCoordPtr[i]->size;
       tstride[i] = VB->TexCoordPtr[i]->stride;
    }
    else tc[i] = 0;
      }
   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      if (VB->SecondaryColorPtr[0]->Type != 0x1401)
    import_float_spec_colors( ctx );

      spec = (GLchan *) VB->SecondaryColorPtr[0]->Ptr;
      spec_stride = VB->SecondaryColorPtr[0]->StrideB;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));


    GLuint u;
    for (u = 0 ; u < maxtex ; u++)
       if (tc[u]) {
          { { v->texcoord[u][0] = 0; v->texcoord[u][1] = 0; v->texcoord[u][2] = 0; v->texcoord[u][3] = 1; }; { switch (tsz[u]) { case 4: (v->texcoord[u])[3] = (tc[u])[3]; case 3: (v->texcoord[u])[2] = (tc[u])[2]; case 2: (v->texcoord[u])[1] = (tc[u])[1]; case 1: (v->texcoord[u])[0] = (tc[u])[0]; } }; };
          (tc[u] = (GLfloat *)((GLubyte *)tc[u] + tstride[u]));
       }

    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));

    { (v->specular)[0] = (spec)[0]; (v->specular)[1] = (spec)[1]; (v->specular)[2] = (spec)[2]; (v->specular)[3] = (spec)[3]; };
    (spec = (GLchan *)((GLubyte *)spec + spec_stride));

    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));


   }
}



static void interp_color_multitex_spec_fog( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }


      GLuint u;
      GLuint maxtex = ctx->Const.MaxTextureUnits;
      for (u = 0 ; u < maxtex ; u++)
    if (VB->TexCoordPtr[u]) {
       { dst->texcoord[u][0] = (((out->texcoord[u])[0]) + ((t)) * (((in->texcoord[u])[0]) - ((out->texcoord[u])[0]))); dst->texcoord[u][1] = (((out->texcoord[u])[1]) + ((t)) * (((in->texcoord[u])[1]) - ((out->texcoord[u])[1]))); dst->texcoord[u][2] = (((out->texcoord[u])[2]) + ((t)) * (((in->texcoord[u])[2]) - ((out->texcoord[u])[2]))); dst->texcoord[u][3] = (((out->texcoord[u])[3]) + ((t)) * (((in->texcoord[u])[3]) - ((out->texcoord[u])[3]))); };
    }

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));


}


static void copy_pv_color_multitex_spec_fog( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };

      { (dst->specular)[0] = (src->specular)[0]; (dst->specular)[1] = (src->specular)[1]; (dst->specular)[2] = (src->specular)[2]; };

}


static void init_color_multitex_spec_fog( void )
{
   setup_tab[(0x1|0x8|0x10|0x20)] = emit_color_multitex_spec_fog;
   interp_tab[(0x1|0x8|0x10|0x20)] = interp_color_multitex_spec_fog;
   copy_pv_tab[(0x1|0x8|0x10|0x20)] = copy_pv_color_multitex_spec_fog;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));



    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));




    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_color_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }



      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };




   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_color_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };


}


static void init_color_point( void )
{
   setup_tab[(0x1|0x40)] = emit_color_point;
   interp_tab[(0x1|0x40)] = interp_color_point;
   copy_pv_tab[(0x1|0x40)] = copy_pv_color_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_spec_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      if (VB->SecondaryColorPtr[0]->Type != 0x1401)
    import_float_spec_colors( ctx );

      spec = (GLchan *) VB->SecondaryColorPtr[0]->Ptr;
      spec_stride = VB->SecondaryColorPtr[0]->StrideB;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));



    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));

    { (v->specular)[0] = (spec)[0]; (v->specular)[1] = (spec)[1]; (v->specular)[2] = (spec)[2]; (v->specular)[3] = (spec)[3]; };
    (spec = (GLchan *)((GLubyte *)spec + spec_stride));



    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_color_spec_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }



      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };



   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_color_spec_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };

      { (dst->specular)[0] = (src->specular)[0]; (dst->specular)[1] = (src->specular)[1]; (dst->specular)[2] = (src->specular)[2]; };

}


static void init_color_spec_point( void )
{
   setup_tab[(0x1|0x10|0x40)] = emit_color_spec_point;
   interp_tab[(0x1|0x10|0x40)] = interp_color_spec_point;
   copy_pv_tab[(0x1|0x10|0x40)] = copy_pv_color_spec_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_fog_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));



    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));


    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));


    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_color_fog_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }



      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };


      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));


   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_color_fog_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };


}


static void init_color_fog_point( void )
{
   setup_tab[(0x1|0x20|0x40)] = emit_color_fog_point;
   interp_tab[(0x1|0x20|0x40)] = interp_color_fog_point;
   copy_pv_tab[(0x1|0x20|0x40)] = copy_pv_color_fog_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_spec_fog_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      if (VB->SecondaryColorPtr[0]->Type != 0x1401)
    import_float_spec_colors( ctx );

      spec = (GLchan *) VB->SecondaryColorPtr[0]->Ptr;
      spec_stride = VB->SecondaryColorPtr[0]->StrideB;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));



    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));

    { (v->specular)[0] = (spec)[0]; (v->specular)[1] = (spec)[1]; (v->specular)[2] = (spec)[2]; (v->specular)[3] = (spec)[3]; };
    (spec = (GLchan *)((GLubyte *)spec + spec_stride));

    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));


    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_color_spec_fog_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }



      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));


   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_color_spec_fog_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };

      { (dst->specular)[0] = (src->specular)[0]; (dst->specular)[1] = (src->specular)[1]; (dst->specular)[2] = (src->specular)[2]; };

}


static void init_color_spec_fog_point( void )
{
   setup_tab[(0x1|0x10|0x20|0x40)] = emit_color_spec_fog_point;
   interp_tab[(0x1|0x10|0x20|0x40)] = interp_color_spec_fog_point;
   copy_pv_tab[(0x1|0x10|0x20|0x40)] = copy_pv_color_spec_fog_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_tex0_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;

//   if (IND & TEX0) {
      tc[0] = (GLfloat *)VB->TexCoordPtr[0]->data;
      tsz[0] = VB->TexCoordPtr[0]->size;
      tstride[0] = VB->TexCoordPtr[0]->stride;
//   }

   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));

    { { v->texcoord[0][0] = 0; v->texcoord[0][1] = 0; v->texcoord[0][2] = 0; v->texcoord[0][3] = 1; }; { switch (tsz[0]) { case 4: (v->texcoord[0])[3] = (tc[0])[3]; case 3: (v->texcoord[0])[2] = (tc[0])[2]; case 2: (v->texcoord[0])[1] = (tc[0])[1]; case 1: (v->texcoord[0])[0] = (tc[0])[0]; } }; };
    (tc[0] = (GLfloat *)((GLubyte *)tc[0] + tstride[0]));


    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));




    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_color_tex0_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }

      { dst->texcoord[0][0] = (((out->texcoord[0])[0]) + ((t)) * (((in->texcoord[0])[0]) - ((out->texcoord[0])[0]))); dst->texcoord[0][1] = (((out->texcoord[0])[1]) + ((t)) * (((in->texcoord[0])[1]) - ((out->texcoord[0])[1]))); dst->texcoord[0][2] = (((out->texcoord[0])[2]) + ((t)) * (((in->texcoord[0])[2]) - ((out->texcoord[0])[2]))); dst->texcoord[0][3] = (((out->texcoord[0])[3]) + ((t)) * (((in->texcoord[0])[3]) - ((out->texcoord[0])[3]))); };


      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };




   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_color_tex0_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };


}


static void init_color_tex0_point( void )
{
   setup_tab[(0x1|0x4|0x40)] = emit_color_tex0_point;
   interp_tab[(0x1|0x4|0x40)] = interp_color_tex0_point;
   copy_pv_tab[(0x1|0x4|0x40)] = copy_pv_color_tex0_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_tex0_spec_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;

//   if (IND & TEX0) {
      tc[0] = (GLfloat *)VB->TexCoordPtr[0]->data;
      tsz[0] = VB->TexCoordPtr[0]->size;
      tstride[0] = VB->TexCoordPtr[0]->stride;
//   }

   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      if (VB->SecondaryColorPtr[0]->Type != 0x1401)
    import_float_spec_colors( ctx );

      spec = (GLchan *) VB->SecondaryColorPtr[0]->Ptr;
      spec_stride = VB->SecondaryColorPtr[0]->StrideB;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));

    { { v->texcoord[0][0] = 0; v->texcoord[0][1] = 0; v->texcoord[0][2] = 0; v->texcoord[0][3] = 1; }; { switch (tsz[0]) { case 4: (v->texcoord[0])[3] = (tc[0])[3]; case 3: (v->texcoord[0])[2] = (tc[0])[2]; case 2: (v->texcoord[0])[1] = (tc[0])[1]; case 1: (v->texcoord[0])[0] = (tc[0])[0]; } }; };
    (tc[0] = (GLfloat *)((GLubyte *)tc[0] + tstride[0]));


    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));

    { (v->specular)[0] = (spec)[0]; (v->specular)[1] = (spec)[1]; (v->specular)[2] = (spec)[2]; (v->specular)[3] = (spec)[3]; };
    (spec = (GLchan *)((GLubyte *)spec + spec_stride));



    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_color_tex0_spec_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }

      { dst->texcoord[0][0] = (((out->texcoord[0])[0]) + ((t)) * (((in->texcoord[0])[0]) - ((out->texcoord[0])[0]))); dst->texcoord[0][1] = (((out->texcoord[0])[1]) + ((t)) * (((in->texcoord[0])[1]) - ((out->texcoord[0])[1]))); dst->texcoord[0][2] = (((out->texcoord[0])[2]) + ((t)) * (((in->texcoord[0])[2]) - ((out->texcoord[0])[2]))); dst->texcoord[0][3] = (((out->texcoord[0])[3]) + ((t)) * (((in->texcoord[0])[3]) - ((out->texcoord[0])[3]))); };


      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };



   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_color_tex0_spec_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };

      { (dst->specular)[0] = (src->specular)[0]; (dst->specular)[1] = (src->specular)[1]; (dst->specular)[2] = (src->specular)[2]; };

}


static void init_color_tex0_spec_point( void )
{
   setup_tab[(0x1|0x4|0x10|0x40)] = emit_color_tex0_spec_point;
   interp_tab[(0x1|0x4|0x10|0x40)] = interp_color_tex0_spec_point;
   copy_pv_tab[(0x1|0x4|0x10|0x40)] = copy_pv_color_tex0_spec_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_tex0_fog_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;

//   if (IND & TEX0) {
      tc[0] = (GLfloat *)VB->TexCoordPtr[0]->data;
      tsz[0] = VB->TexCoordPtr[0]->size;
      tstride[0] = VB->TexCoordPtr[0]->stride;
//   }

   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));

    { { v->texcoord[0][0] = 0; v->texcoord[0][1] = 0; v->texcoord[0][2] = 0; v->texcoord[0][3] = 1; }; { switch (tsz[0]) { case 4: (v->texcoord[0])[3] = (tc[0])[3]; case 3: (v->texcoord[0])[2] = (tc[0])[2]; case 2: (v->texcoord[0])[1] = (tc[0])[1]; case 1: (v->texcoord[0])[0] = (tc[0])[0]; } }; };
    (tc[0] = (GLfloat *)((GLubyte *)tc[0] + tstride[0]));


    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));


    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));


    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_color_tex0_fog_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }

      { dst->texcoord[0][0] = (((out->texcoord[0])[0]) + ((t)) * (((in->texcoord[0])[0]) - ((out->texcoord[0])[0]))); dst->texcoord[0][1] = (((out->texcoord[0])[1]) + ((t)) * (((in->texcoord[0])[1]) - ((out->texcoord[0])[1]))); dst->texcoord[0][2] = (((out->texcoord[0])[2]) + ((t)) * (((in->texcoord[0])[2]) - ((out->texcoord[0])[2]))); dst->texcoord[0][3] = (((out->texcoord[0])[3]) + ((t)) * (((in->texcoord[0])[3]) - ((out->texcoord[0])[3]))); };


      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };


      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));


   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_color_tex0_fog_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };


}


static void init_color_tex0_fog_point( void )
{
   setup_tab[(0x1|0x4|0x20|0x40)] = emit_color_tex0_fog_point;
   interp_tab[(0x1|0x4|0x20|0x40)] = interp_color_tex0_fog_point;
   copy_pv_tab[(0x1|0x4|0x20|0x40)] = copy_pv_color_tex0_fog_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_tex0_spec_fog_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;

//   if (IND & TEX0) {
      tc[0] = (GLfloat *)VB->TexCoordPtr[0]->data;
      tsz[0] = VB->TexCoordPtr[0]->size;
      tstride[0] = VB->TexCoordPtr[0]->stride;
//   }

   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      if (VB->SecondaryColorPtr[0]->Type != 0x1401)
    import_float_spec_colors( ctx );

      spec = (GLchan *) VB->SecondaryColorPtr[0]->Ptr;
      spec_stride = VB->SecondaryColorPtr[0]->StrideB;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));

    { { v->texcoord[0][0] = 0; v->texcoord[0][1] = 0; v->texcoord[0][2] = 0; v->texcoord[0][3] = 1; }; { switch (tsz[0]) { case 4: (v->texcoord[0])[3] = (tc[0])[3]; case 3: (v->texcoord[0])[2] = (tc[0])[2]; case 2: (v->texcoord[0])[1] = (tc[0])[1]; case 1: (v->texcoord[0])[0] = (tc[0])[0]; } }; };
    (tc[0] = (GLfloat *)((GLubyte *)tc[0] + tstride[0]));


    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));

    { (v->specular)[0] = (spec)[0]; (v->specular)[1] = (spec)[1]; (v->specular)[2] = (spec)[2]; (v->specular)[3] = (spec)[3]; };
    (spec = (GLchan *)((GLubyte *)spec + spec_stride));

    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));


    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_color_tex0_spec_fog_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }

      { dst->texcoord[0][0] = (((out->texcoord[0])[0]) + ((t)) * (((in->texcoord[0])[0]) - ((out->texcoord[0])[0]))); dst->texcoord[0][1] = (((out->texcoord[0])[1]) + ((t)) * (((in->texcoord[0])[1]) - ((out->texcoord[0])[1]))); dst->texcoord[0][2] = (((out->texcoord[0])[2]) + ((t)) * (((in->texcoord[0])[2]) - ((out->texcoord[0])[2]))); dst->texcoord[0][3] = (((out->texcoord[0])[3]) + ((t)) * (((in->texcoord[0])[3]) - ((out->texcoord[0])[3]))); };


      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));


   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_color_tex0_spec_fog_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };

      { (dst->specular)[0] = (src->specular)[0]; (dst->specular)[1] = (src->specular)[1]; (dst->specular)[2] = (src->specular)[2]; };

}


static void init_color_tex0_spec_fog_point( void )
{
   setup_tab[(0x1|0x4|0x10|0x20|0x40)] = emit_color_tex0_spec_fog_point;
   interp_tab[(0x1|0x4|0x10|0x20|0x40)] = interp_color_tex0_spec_fog_point;
   copy_pv_tab[(0x1|0x4|0x10|0x20|0x40)] = copy_pv_color_tex0_spec_fog_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_multitex_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


      for (i = 0 ; i < ctx->Const.MaxTextureUnits ; i++) {
    if (VB->TexCoordPtr[i]) {
       maxtex = i+1;
       tc[i] = (GLfloat *)VB->TexCoordPtr[i]->data;
       tsz[i] = VB->TexCoordPtr[i]->size;
       tstride[i] = VB->TexCoordPtr[i]->stride;
    }
    else tc[i] = 0;
      }
   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));


    GLuint u;
    for (u = 0 ; u < maxtex ; u++)
       if (tc[u]) {
          { { v->texcoord[u][0] = 0; v->texcoord[u][1] = 0; v->texcoord[u][2] = 0; v->texcoord[u][3] = 1; }; { switch (tsz[u]) { case 4: (v->texcoord[u])[3] = (tc[u])[3]; case 3: (v->texcoord[u])[2] = (tc[u])[2]; case 2: (v->texcoord[u])[1] = (tc[u])[1]; case 1: (v->texcoord[u])[0] = (tc[u])[0]; } }; };
          (tc[u] = (GLfloat *)((GLubyte *)tc[u] + tstride[u]));
       }

    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));




    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_color_multitex_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }


      GLuint u;
      GLuint maxtex = ctx->Const.MaxTextureUnits;
      for (u = 0 ; u < maxtex ; u++)
    if (VB->TexCoordPtr[u]) {
       { dst->texcoord[u][0] = (((out->texcoord[u])[0]) + ((t)) * (((in->texcoord[u])[0]) - ((out->texcoord[u])[0]))); dst->texcoord[u][1] = (((out->texcoord[u])[1]) + ((t)) * (((in->texcoord[u])[1]) - ((out->texcoord[u])[1]))); dst->texcoord[u][2] = (((out->texcoord[u])[2]) + ((t)) * (((in->texcoord[u])[2]) - ((out->texcoord[u])[2]))); dst->texcoord[u][3] = (((out->texcoord[u])[3]) + ((t)) * (((in->texcoord[u])[3]) - ((out->texcoord[u])[3]))); };
    }

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };




   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_color_multitex_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };


}


static void init_color_multitex_point( void )
{
   setup_tab[(0x1|0x8|0x40)] = emit_color_multitex_point;
   interp_tab[(0x1|0x8|0x40)] = interp_color_multitex_point;
   copy_pv_tab[(0x1|0x8|0x40)] = copy_pv_color_multitex_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_multitex_spec_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


      for (i = 0 ; i < ctx->Const.MaxTextureUnits ; i++) {
    if (VB->TexCoordPtr[i]) {
       maxtex = i+1;
       tc[i] = (GLfloat *)VB->TexCoordPtr[i]->data;
       tsz[i] = VB->TexCoordPtr[i]->size;
       tstride[i] = VB->TexCoordPtr[i]->stride;
    }
    else tc[i] = 0;
      }
   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      if (VB->SecondaryColorPtr[0]->Type != 0x1401)
    import_float_spec_colors( ctx );

      spec = (GLchan *) VB->SecondaryColorPtr[0]->Ptr;
      spec_stride = VB->SecondaryColorPtr[0]->StrideB;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));


    GLuint u;
    for (u = 0 ; u < maxtex ; u++)
       if (tc[u]) {
          { { v->texcoord[u][0] = 0; v->texcoord[u][1] = 0; v->texcoord[u][2] = 0; v->texcoord[u][3] = 1; }; { switch (tsz[u]) { case 4: (v->texcoord[u])[3] = (tc[u])[3]; case 3: (v->texcoord[u])[2] = (tc[u])[2]; case 2: (v->texcoord[u])[1] = (tc[u])[1]; case 1: (v->texcoord[u])[0] = (tc[u])[0]; } }; };
          (tc[u] = (GLfloat *)((GLubyte *)tc[u] + tstride[u]));
       }

    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));

    { (v->specular)[0] = (spec)[0]; (v->specular)[1] = (spec)[1]; (v->specular)[2] = (spec)[2]; (v->specular)[3] = (spec)[3]; };
    (spec = (GLchan *)((GLubyte *)spec + spec_stride));



    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_color_multitex_spec_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }


      GLuint u;
      GLuint maxtex = ctx->Const.MaxTextureUnits;
      for (u = 0 ; u < maxtex ; u++)
    if (VB->TexCoordPtr[u]) {
       { dst->texcoord[u][0] = (((out->texcoord[u])[0]) + ((t)) * (((in->texcoord[u])[0]) - ((out->texcoord[u])[0]))); dst->texcoord[u][1] = (((out->texcoord[u])[1]) + ((t)) * (((in->texcoord[u])[1]) - ((out->texcoord[u])[1]))); dst->texcoord[u][2] = (((out->texcoord[u])[2]) + ((t)) * (((in->texcoord[u])[2]) - ((out->texcoord[u])[2]))); dst->texcoord[u][3] = (((out->texcoord[u])[3]) + ((t)) * (((in->texcoord[u])[3]) - ((out->texcoord[u])[3]))); };
    }

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };



   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_color_multitex_spec_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };

      { (dst->specular)[0] = (src->specular)[0]; (dst->specular)[1] = (src->specular)[1]; (dst->specular)[2] = (src->specular)[2]; };

}


static void init_color_multitex_spec_point( void )
{
   setup_tab[(0x1|0x8|0x10|0x40)] = emit_color_multitex_spec_point;
   interp_tab[(0x1|0x8|0x10|0x40)] = interp_color_multitex_spec_point;
   copy_pv_tab[(0x1|0x8|0x10|0x40)] = copy_pv_color_multitex_spec_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_multitex_fog_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


      for (i = 0 ; i < ctx->Const.MaxTextureUnits ; i++) {
    if (VB->TexCoordPtr[i]) {
       maxtex = i+1;
       tc[i] = (GLfloat *)VB->TexCoordPtr[i]->data;
       tsz[i] = VB->TexCoordPtr[i]->size;
       tstride[i] = VB->TexCoordPtr[i]->stride;
    }
    else tc[i] = 0;
      }
   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));


    GLuint u;
    for (u = 0 ; u < maxtex ; u++)
       if (tc[u]) {
          { { v->texcoord[u][0] = 0; v->texcoord[u][1] = 0; v->texcoord[u][2] = 0; v->texcoord[u][3] = 1; }; { switch (tsz[u]) { case 4: (v->texcoord[u])[3] = (tc[u])[3]; case 3: (v->texcoord[u])[2] = (tc[u])[2]; case 2: (v->texcoord[u])[1] = (tc[u])[1]; case 1: (v->texcoord[u])[0] = (tc[u])[0]; } }; };
          (tc[u] = (GLfloat *)((GLubyte *)tc[u] + tstride[u]));
       }

    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));


    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));


    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_color_multitex_fog_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }


      GLuint u;
      GLuint maxtex = ctx->Const.MaxTextureUnits;
      for (u = 0 ; u < maxtex ; u++)
    if (VB->TexCoordPtr[u]) {
       { dst->texcoord[u][0] = (((out->texcoord[u])[0]) + ((t)) * (((in->texcoord[u])[0]) - ((out->texcoord[u])[0]))); dst->texcoord[u][1] = (((out->texcoord[u])[1]) + ((t)) * (((in->texcoord[u])[1]) - ((out->texcoord[u])[1]))); dst->texcoord[u][2] = (((out->texcoord[u])[2]) + ((t)) * (((in->texcoord[u])[2]) - ((out->texcoord[u])[2]))); dst->texcoord[u][3] = (((out->texcoord[u])[3]) + ((t)) * (((in->texcoord[u])[3]) - ((out->texcoord[u])[3]))); };
    }

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };


      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));


   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_color_multitex_fog_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };


}


static void init_color_multitex_fog_point( void )
{
   setup_tab[(0x1|0x8|0x20|0x40)] = emit_color_multitex_fog_point;
   interp_tab[(0x1|0x8|0x20|0x40)] = interp_color_multitex_fog_point;
   copy_pv_tab[(0x1|0x8|0x20|0x40)] = copy_pv_color_multitex_fog_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_color_multitex_spec_fog_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


      for (i = 0 ; i < ctx->Const.MaxTextureUnits ; i++) {
    if (VB->TexCoordPtr[i]) {
       maxtex = i+1;
       tc[i] = (GLfloat *)VB->TexCoordPtr[i]->data;
       tsz[i] = VB->TexCoordPtr[i]->size;
       tstride[i] = VB->TexCoordPtr[i]->stride;
    }
    else tc[i] = 0;
      }
   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      if (VB->ColorPtr[0]->Type != 0x1401)
    import_float_colors( ctx );

      color = (GLchan *) VB->ColorPtr[0]->Ptr;
      color_stride = VB->ColorPtr[0]->StrideB;
      if (VB->SecondaryColorPtr[0]->Type != 0x1401)
    import_float_spec_colors( ctx );

      spec = (GLchan *) VB->SecondaryColorPtr[0]->Ptr;
      spec_stride = VB->SecondaryColorPtr[0]->StrideB;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));


    GLuint u;
    for (u = 0 ; u < maxtex ; u++)
       if (tc[u]) {
          { { v->texcoord[u][0] = 0; v->texcoord[u][1] = 0; v->texcoord[u][2] = 0; v->texcoord[u][3] = 1; }; { switch (tsz[u]) { case 4: (v->texcoord[u])[3] = (tc[u])[3]; case 3: (v->texcoord[u])[2] = (tc[u])[2]; case 2: (v->texcoord[u])[1] = (tc[u])[1]; case 1: (v->texcoord[u])[0] = (tc[u])[0]; } }; };
          (tc[u] = (GLfloat *)((GLubyte *)tc[u] + tstride[u]));
       }

    { (v->color)[0] = (color)[0]; (v->color)[1] = (color)[1]; (v->color)[2] = (color)[2]; (v->color)[3] = (color)[3]; };
    (color = (GLchan *)((GLubyte *)color + color_stride));

    { (v->specular)[0] = (spec)[0]; (v->specular)[1] = (spec)[1]; (v->specular)[2] = (spec)[2]; (v->specular)[3] = (spec)[3]; };
    (spec = (GLchan *)((GLubyte *)spec + spec_stride));

    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));


    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_color_multitex_spec_fog_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }


      GLuint u;
      GLuint maxtex = ctx->Const.MaxTextureUnits;
      for (u = 0 ; u < maxtex ; u++)
    if (VB->TexCoordPtr[u]) {
       { dst->texcoord[u][0] = (((out->texcoord[u])[0]) + ((t)) * (((in->texcoord[u])[0]) - ((out->texcoord[u])[0]))); dst->texcoord[u][1] = (((out->texcoord[u])[1]) + ((t)) * (((in->texcoord[u])[1]) - ((out->texcoord[u])[1]))); dst->texcoord[u][2] = (((out->texcoord[u])[2]) + ((t)) * (((in->texcoord[u])[2]) - ((out->texcoord[u])[2]))); dst->texcoord[u][3] = (((out->texcoord[u])[3]) + ((t)) * (((in->texcoord[u])[3]) - ((out->texcoord[u])[3]))); };
    }

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->color[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->color[3])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->color[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[0])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[1])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };
      { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(in->specular[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(out->specular[2])]; GLfloat dstf = ((outf) + (t) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); dst->specular[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; };

      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));


   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_color_multitex_spec_fog_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];

      { (dst->color)[0] = (src->color)[0]; (dst->color)[1] = (src->color)[1]; (dst->color)[2] = (src->color)[2]; (dst->color)[3] = (src->color)[3]; };

      { (dst->specular)[0] = (src->specular)[0]; (dst->specular)[1] = (src->specular)[1]; (dst->specular)[2] = (src->specular)[2]; };

}


static void init_color_multitex_spec_fog_point( void )
{
   setup_tab[(0x1|0x8|0x10|0x20|0x40)] = emit_color_multitex_spec_fog_point;
   interp_tab[(0x1|0x8|0x10|0x20|0x40)] = interp_color_multitex_spec_fog_point;
   copy_pv_tab[(0x1|0x8|0x10|0x20|0x40)] = copy_pv_color_multitex_spec_fog_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_index(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      index = VB->IndexPtr[0]->data;
      index_stride = VB->IndexPtr[0]->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));






    v->index = index[0];
    (index = (GLuint *)((GLubyte *)index + index_stride));

   }
}



static void interp_index( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }






      dst->index = (GLuint) (GLint) (((GLfloat) (out->index)) + ((t)) * (((GLfloat) (in->index)) - ((GLfloat) (out->index))));

}


static void copy_pv_index( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];



      dst->index = src->index;
}


static void init_index( void )
{
   setup_tab[(0x2)] = emit_index;
   interp_tab[(0x2)] = interp_index;
   copy_pv_tab[(0x2)] = copy_pv_index;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_index_fog(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      index = VB->IndexPtr[0]->data;
      index_stride = VB->IndexPtr[0]->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));





    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));

    v->index = index[0];
    (index = (GLuint *)((GLubyte *)index + index_stride));

   }
}



static void interp_index_fog( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }





      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));

      dst->index = (GLuint) (GLint) (((GLfloat) (out->index)) + ((t)) * (((GLfloat) (in->index)) - ((GLfloat) (out->index))));

}


static void copy_pv_index_fog( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];



      dst->index = src->index;
}


static void init_index_fog( void )
{
   setup_tab[(0x2|0x20)] = emit_index_fog;
   interp_tab[(0x2|0x20)] = interp_index_fog;
   copy_pv_tab[(0x2|0x20)] = copy_pv_index_fog;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_index_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      index = VB->IndexPtr[0]->data;
      index_stride = VB->IndexPtr[0]->stride;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));






    v->index = index[0];
    (index = (GLuint *)((GLubyte *)index + index_stride));

    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_index_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }






      dst->index = (GLuint) (GLint) (((GLfloat) (out->index)) + ((t)) * (((GLfloat) (in->index)) - ((GLfloat) (out->index))));

   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_index_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];



      dst->index = src->index;
}


static void init_index_point( void )
{
   setup_tab[(0x2|0x40)] = emit_index_point;
   interp_tab[(0x2|0x40)] = interp_index_point;
   copy_pv_tab[(0x2|0x40)] = copy_pv_index_point;
}


/* $Id: ss_vbtmp.h,v 1.22 2002/10/29 20:29:01 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  4.1
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


static void emit_index_fog_point(GLcontext *ctx, GLuint start, GLuint end,
             GLuint newinputs )
{
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   struct vertex_buffer *VB = &tnl->vb;
   SWvertex *v;
   GLfloat *proj;      /* projected clip coordinates */
   GLfloat *tc[8];
   GLchan *color;
   GLchan *spec;
   GLuint *index;
   GLfloat *fog;
   GLfloat *pointSize;
   GLuint tsz[8];
   GLuint tstride[8];
   GLuint proj_stride, color_stride, spec_stride, index_stride;
   GLuint fog_stride, pointSize_stride;
   GLuint i;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   const GLfloat sx = m[0];
   const GLfloat sy = m[5];
   const GLfloat sz = m[10];
   const GLfloat tx = m[12];
   const GLfloat ty = m[13];
   const GLfloat tz = m[14];
   GLuint maxtex = 0;


   proj = VB->NdcPtr->data[0];
   proj_stride = VB->NdcPtr->stride;

      fog = (GLfloat *) VB->FogCoordPtr->data;
      fog_stride = VB->FogCoordPtr->stride;
      index = VB->IndexPtr[0]->data;
      index_stride = VB->IndexPtr[0]->stride;
      pointSize = (GLfloat *) VB->PointSizePtr->data;
      pointSize_stride = VB->PointSizePtr->stride;

   v = &(((SScontext *)ctx->swsetup_context)->verts[start]);

   for (i=start; i < end; i++, v++) {
      if (VB->ClipMask[i] == 0) {
    v->win[0] = sx * proj[0] + tx;
    v->win[1] = sy * proj[1] + ty;
    v->win[2] = sz * proj[2] + tz;
    v->win[3] =      proj[3];
      }
      (proj = (GLfloat *)((GLubyte *)proj + proj_stride));





    v->fog = fog[0];
    (fog = (GLfloat *)((GLubyte *)fog + fog_stride));

    v->index = index[0];
    (index = (GLuint *)((GLubyte *)index + index_stride));

    v->pointSize = pointSize[0];
    (pointSize = (GLfloat *)((GLubyte *)pointSize + pointSize_stride));
   }
}



static void interp_index_fog_point( GLcontext *ctx,
            GLfloat t,
            GLuint edst, GLuint eout, GLuint ein,
            GLboolean force_boundary )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
   GLfloat *m = ctx->Viewport._WindowMap.m;
   GLfloat *clip = VB->ClipPtr->data[edst];

   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *in  = &swsetup->verts[ein];
   SWvertex *out = &swsetup->verts[eout];

   /* Avoid division by zero by rearranging order of clip planes?
    */
   if (clip[3] != 0.0) {
      GLfloat oow = 1.0F / clip[3];
      dst->win[0] = m[0]  * clip[0] * oow + m[12];
      dst->win[1] = m[5]  * clip[1] * oow + m[13];
      dst->win[2] = m[10] * clip[2] * oow + m[14];
      dst->win[3] =                   oow;
   }





      dst->fog = ((out->fog) + (t) * ((in->fog) - (out->fog)));

      dst->index = (GLuint) (GLint) (((GLfloat) (out->index)) + ((t)) * (((GLfloat) (in->index)) - ((GLfloat) (out->index))));

   /* XXX Point size interpolation??? */
      dst->pointSize = ((out->pointSize) + (t) * ((in->pointSize) - (out->pointSize)));
}


static void copy_pv_index_fog_point( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   SWvertex *dst = &swsetup->verts[edst];
   SWvertex *src = &swsetup->verts[esrc];



      dst->index = src->index;
}


static void init_index_fog_point( void )
{
   setup_tab[(0x2|0x20|0x40)] = emit_index_fog_point;
   interp_tab[(0x2|0x20|0x40)] = interp_index_fog_point;
   copy_pv_tab[(0x2|0x20|0x40)] = copy_pv_index_fog_point;
}



/***********************************************************************
 *      Additional setup and interp for back color and edgeflag.
 ***********************************************************************/


static void interp_extras( GLcontext *ctx,
              GLfloat t,
              GLuint dst, GLuint out, GLuint in,
              GLboolean force_boundary )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;

   if (VB->ColorPtr[1]) {
      { { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[in]))[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[out]))[0])]; GLfloat dstf = ((outf) + ((t)) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); ((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[dst]))[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; }; { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[in]))[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[out]))[1])]; GLfloat dstf = ((outf) + ((t)) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); ((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[dst]))[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; }; { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[in]))[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[out]))[2])]; GLfloat dstf = ((outf) + ((t)) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); ((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[dst]))[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; }; { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[in]))[3])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[out]))[3])]; GLfloat dstf = ((outf) + ((t)) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); ((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[dst]))[3] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; }; };

      if (VB->SecondaryColorPtr[1]) {
    { { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[in]))[0])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[out]))[0])]; GLfloat dstf = ((outf) + ((t)) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); ((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[dst]))[0] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; }; { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[in]))[1])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[out]))[1])]; GLfloat dstf = ((outf) + ((t)) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); ((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[dst]))[1] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; }; { GLfloat inf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[in]))[2])]; GLfloat outf = _mesa_ubyte_to_float_color_tab[(unsigned int)(((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[out]))[2])]; GLfloat dstf = ((outf) + ((t)) * ((inf) - (outf))); { union { GLfloat r; GLuint i; } __tmp; __tmp.r = (dstf); ((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[dst]))[2] = ((__tmp.i >= 0x3f7f0000) ? ((GLint)__tmp.i < 0) ? (GLubyte)0 : (GLubyte)255 : (__tmp.r = __tmp.r*(255.0F/256.0F) + 32768.0F, (GLubyte)__tmp.i)); }; }; };
      }
   }
   else if (VB->IndexPtr[1]) {
      VB->IndexPtr[1]->data[dst] = (GLuint) (GLint)
    (((GLfloat) VB->IndexPtr[1]->data[out]) + (t) * (((GLfloat) VB->IndexPtr[1]->data[in]) - ((GLfloat) VB->IndexPtr[1]->data[out])));
   }

   if (VB->EdgeFlag) {
      VB->EdgeFlag[dst] = VB->EdgeFlag[out] || force_boundary;
   }

   interp_tab[((SScontext *)ctx->swsetup_context)->SetupIndex](ctx, t, dst, out, in,
                       force_boundary);
}

static void copy_pv_extras( GLcontext *ctx, GLuint dst, GLuint src )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;

   if (VB->ColorPtr[1]) {
    { ((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[dst]))[0] = ((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[src]))[0]; ((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[dst]))[1] = ((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[src]))[1]; ((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[dst]))[2] = ((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[src]))[2]; ((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[dst]))[3] = ((((GLchan (*)[4])((VB->ColorPtr[1])->Ptr))[src]))[3]; };

    if (VB->SecondaryColorPtr[1]) {
       { ((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[dst]))[0] = ((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[src]))[0]; ((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[dst]))[1] = ((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[src]))[1]; ((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[dst]))[2] = ((((GLchan (*)[4])((VB->SecondaryColorPtr[1])->Ptr))[src]))[2]; };
    }
   }
   else if (VB->IndexPtr[1]) {
      VB->IndexPtr[1]->data[dst] = VB->IndexPtr[1]->data[src];
   }

   copy_pv_tab[((SScontext *)ctx->swsetup_context)->SetupIndex](ctx, dst, src);
}




/***********************************************************************
 *                         Initialization
 ***********************************************************************/

static void
emit_invalid( GLcontext *ctx, GLuint start, GLuint end, GLuint newinputs )
{
   _mesa_debug(ctx, "swrast_setup: invalid setup function\n");
   (void) (ctx && start && end && newinputs);
}


static void
interp_invalid( GLcontext *ctx, GLfloat t,
       GLuint edst, GLuint eout, GLuint ein,
       GLboolean force_boundary )
{
   _mesa_debug(ctx, "swrast_setup: invalid interp function\n");
   (void) (ctx && t && edst && eout && ein && force_boundary);
}


static void
copy_pv_invalid( GLcontext *ctx, GLuint edst, GLuint esrc )
{
   _mesa_debug(ctx, "swrast_setup: invalid copy_pv function\n");
   (void) (ctx && edst && esrc );
}


static void init_standard( void )
{
   GLuint i;

   for (i = 0 ; i < sizeof(setup_tab)/sizeof(*(setup_tab)) ; i++) {
      setup_tab[i] = emit_invalid;
      interp_tab[i] = interp_invalid;
      copy_pv_tab[i] = copy_pv_invalid;
   }

   init_none();
   init_color();
   init_color_spec();
   init_color_fog();
   init_color_spec_fog();
   init_color_tex0();
   init_color_tex0_spec();
   init_color_tex0_fog();
   init_color_tex0_spec_fog();
   init_color_multitex();
   init_color_multitex_spec();
   init_color_multitex_fog();
   init_color_multitex_spec_fog();
   init_color_point();
   init_color_spec_point();
   init_color_fog_point();
   init_color_spec_fog_point();
   init_color_tex0_point();
   init_color_tex0_spec_point();
   init_color_tex0_fog_point();
   init_color_tex0_spec_fog_point();
   init_color_multitex_point();
   init_color_multitex_spec_point();
   init_color_multitex_fog_point();
   init_color_multitex_spec_fog_point();
   init_index();
   init_index_fog();
   init_index_point();
   init_index_fog_point();
}


/* debug only */

void
_swsetup_choose_rastersetup_func(GLcontext *ctx)
{
   SScontext *swsetup = ((SScontext *)ctx->swsetup_context);
   TNLcontext *tnl = ((TNLcontext *)(ctx->swtnl_context));
   int funcindex = 0;

   if (ctx->RenderMode == 0x1C00) {
      if (ctx->Visual.rgbMode) {
         funcindex = 0x1;

         if (ctx->Texture._EnabledUnits > 1)
            funcindex |= 0x8; /* a unit above unit[0] is enabled */
         else if (ctx->Texture._EnabledUnits == 1)
            funcindex |= 0x4;  /* only unit 0 is enabled */

         if (ctx->_TriangleCaps & 0x2)
            funcindex |= 0x10;
      }
      else {
         funcindex = 0x2;
      }

      if (ctx->Point._Attenuated ||
          (ctx->VertexProgram.Enabled && ctx->VertexProgram.PointSizeEnabled))
         funcindex |= 0x40;

      if (ctx->Fog.Enabled)
    funcindex |= 0x20;
   }
   else if (ctx->RenderMode == 0x1C01) {
      if (ctx->Visual.rgbMode)
    funcindex = (0x1 | 0x4); /* is feedback color subject to fogging? */
      else
    funcindex = 0x2;
   }
   else
      funcindex = 0;

   swsetup->SetupIndex = funcindex;
   tnl->Driver.Render.BuildVertices = setup_tab[funcindex];

   if (ctx->_TriangleCaps & (0x8|0x10)) {
      tnl->Driver.Render.Interp = interp_extras;
      tnl->Driver.Render.CopyPV = copy_pv_extras;
   }
   else {
      tnl->Driver.Render.Interp = interp_tab[funcindex];
      tnl->Driver.Render.CopyPV = copy_pv_tab[funcindex];
   }

   ;
   ;
}


void
_swsetup_vb_init( GLcontext *ctx )
{
   (void) ctx;
   init_standard();
   /*
   printSetupFlags(ctx);
   */
}




