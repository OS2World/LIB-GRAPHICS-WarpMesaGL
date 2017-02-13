/* $Id: s_context.c,v 1.42 2002/10/29 20:28:59 brianp Exp $ */


#include "glheader.h"
#include "context.h"
#include "mtypes.h"
#include "imports.h"

#include "swrast.h"
#include "s_blend.h"
#include "s_context.h"
#include "s_lines.h"
#include "s_points.h"
#include "s_span.h"
#include "s_triangle.h"
#include "s_texture.h"


/*
 * Recompute the value of swrast->_RasterMask, etc. according to
 * the current context.
 */
static void
_swrast_update_rasterflags( GLcontext *ctx )
{
   GLuint RasterMask = 0;

   if (ctx->Color.AlphaEnabled)           RasterMask |= 0x001;
   if (ctx->Color.BlendEnabled)           RasterMask |= 0x002;
   if (ctx->Depth.Test)                   RasterMask |= 0x004;
   if (ctx->Fog.Enabled)                  RasterMask |= 0x008;
   if (ctx->Scissor.Enabled)              RasterMask |= 0x020;
   if (ctx->Stencil.Enabled)              RasterMask |= 0x040;
   if (ctx->Visual.rgbMode) {
      const GLuint colorMask = *((GLuint *) &ctx->Color.ColorMask);
      if (colorMask != 0xffffffff)        RasterMask |= 0x080;
      if (ctx->Color.ColorLogicOpEnabled) RasterMask |= 0x010;
      if (ctx->Texture._EnabledUnits)     RasterMask |= 0x1000;
   }
   else {
      if (ctx->Color.IndexMask != 0xffffffff) RasterMask |= 0x080;
      if (ctx->Color.IndexLogicOpEnabled)     RasterMask |= 0x010;
   }

   if (ctx->DrawBuffer->UseSoftwareAlphaBuffers
       && ctx->Color.ColorMask[3]
       && ctx->Color.DrawBuffer != 0x0)
      RasterMask |= 0x100;

   if (   ctx->Viewport.X < 0
       || ctx->Viewport.X + ctx->Viewport.Width > (GLint) ctx->DrawBuffer->Width
       || ctx->Viewport.Y < 0
       || ctx->Viewport.Y + ctx->Viewport.Height > (GLint) ctx->DrawBuffer->Height) {
      RasterMask |= 0x020;
   }

   if (ctx->Depth.OcclusionTest)
      RasterMask |= 0x800;


   /* If we're not drawing to exactly one color buffer set the
    * MULTI_DRAW_BIT flag.  Also set it if we're drawing to no
    * buffers or the RGBA or CI mask disables all writes.
    */
   if (ctx->Color._DrawDestMask != 0x1 &&
       ctx->Color._DrawDestMask != 0x4 &&
       ctx->Color._DrawDestMask != 0x2 &&
       ctx->Color._DrawDestMask != 0x8) {
      /* more than one color buffer designated for writing (or zero buffers) */
      RasterMask |= 0x400;
   }
   else if (ctx->Visual.rgbMode && *((GLuint *) ctx->Color.ColorMask) == 0) {
      RasterMask |= 0x400; /* all RGBA channels disabled */
   }
   else if (!ctx->Visual.rgbMode && ctx->Color.IndexMask==0) {
      RasterMask |= 0x400; /* all color index bits disabled */
   }

   ((SWcontext *)ctx->swrast_context)->_RasterMask = RasterMask;
}


static void
_swrast_update_polygon( GLcontext *ctx )
{
   GLfloat backface_sign = 1;

   if (ctx->Polygon.CullFlag) {
      backface_sign = 1;
      switch(ctx->Polygon.CullFaceMode) {
      case 0x0405:
        if(ctx->Polygon.FrontFace==0x0901)
           backface_sign = -1;
        break;
      case 0x0404:
        if(ctx->Polygon.FrontFace!=0x0901)
           backface_sign = -1;
        break;
      default:
      case 0x0408:
        backface_sign = 0;
        break;
      }
   }
   else {
      backface_sign = 0;
   }

   ((SWcontext *)ctx->swrast_context)->_backface_sign = backface_sign;
}


static void
_swrast_update_hint( GLcontext *ctx )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);
   swrast->_PreferPixelFog = (!swrast->AllowVertexFog ||
                             (ctx->Hint.Fog == 0x1102 &&
                              swrast->AllowPixelFog));
}


/*
 * Update the swrast->_AnyTextureCombine flag.
 */
static void
_swrast_update_texture_env( GLcontext *ctx )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);
   GLuint i;
   swrast->_AnyTextureCombine = 0x0;
   for (i = 0; i < ctx->Const.MaxTextureUnits; i++) {
      if (ctx->Texture.Unit[i].EnvMode == 0x8570 ||
          ctx->Texture.Unit[i].EnvMode == 0x8503) {
         swrast->_AnyTextureCombine = 0x1;
         return;
      }
   }
}



/* State referenced by _swrast_choose_triangle, _swrast_choose_line.
 */








/* Stub for swrast->Triangle to select a true triangle function
 * after a state change.
 */
static void
_swrast_validate_triangle( GLcontext *ctx,
                          const SWvertex *v0,
                           const SWvertex *v1,
                           const SWvertex *v2 )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);

   _swrast_validate_derived( ctx );
   swrast->choose_triangle( ctx );

   if ((ctx->_TriangleCaps & 0x2) &&
       ctx->Texture._EnabledUnits == 0) {
      swrast->SpecTriangle = swrast->Triangle;
      swrast->Triangle = _swrast_add_spec_terms_triangle;
   }
// _mesa_debug(ctx, "_swrast_validate_triangle Triangle call\n");

   swrast->Triangle( ctx, v0, v1, v2 );
// _mesa_debug(ctx, "_swrast_validate_triangle  Triangle call return \n");
}

static void
_swrast_validate_line( GLcontext *ctx, const SWvertex *v0, const SWvertex *v1 )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);

   _swrast_validate_derived( ctx );
   swrast->choose_line( ctx );

   if ((ctx->_TriangleCaps & 0x2) &&
       ctx->Texture._EnabledUnits == 0) {
      swrast->SpecLine = swrast->Line;
      swrast->Line = _swrast_add_spec_terms_line;
   }


   swrast->Line( ctx, v0, v1 );
}

static void
_swrast_validate_point( GLcontext *ctx, const SWvertex *v0 )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);

   _swrast_validate_derived( ctx );
   swrast->choose_point( ctx );

   if ((ctx->_TriangleCaps & 0x2) &&
       ctx->Texture._EnabledUnits == 0) {
      swrast->SpecPoint = swrast->Point;
      swrast->Point = _swrast_add_spec_terms_point;
   }

   swrast->Point( ctx, v0 );
}

static void
_swrast_validate_blend_func( GLcontext *ctx, GLuint n,
                            const GLubyte mask[],
                            GLchan src[][4],
                            const GLchan dst[][4] )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);

   _swrast_validate_derived( ctx );
   _swrast_choose_blend_func( ctx );

   swrast->BlendFunc( ctx, n, mask, src, dst );
}


static void
_swrast_validate_texture_sample( GLcontext *ctx, GLuint texUnit,
                                const struct gl_texture_object *tObj,
                                GLuint n, GLfloat texcoords[][4],
                                const GLfloat lambda[], GLchan rgba[][4] )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);

   _swrast_validate_derived( ctx );
   _swrast_choose_texture_sample_func( ctx, texUnit, tObj );

   swrast->TextureSample[texUnit]( ctx, texUnit, tObj, n, texcoords,
                                  lambda, rgba );
}


static void
_swrast_sleep( GLcontext *ctx, GLuint new_state )
{
}


static void
_swrast_invalidate_state( GLcontext *ctx, GLuint new_state )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);
   GLuint i;

   swrast->NewState |= new_state;

   /* After 10 statechanges without any swrast functions being called,
    * put the module to sleep.
    */
   if (++swrast->StateChanges > 10) {
      swrast->InvalidateState = _swrast_sleep;
      swrast->NewState = ~0;
      new_state = ~0;
   }

   if (new_state & swrast->invalidate_triangle)
      swrast->Triangle = _swrast_validate_triangle;

   if (new_state & swrast->invalidate_line)
      swrast->Line = _swrast_validate_line;

   if (new_state & swrast->invalidate_point)
      swrast->Point = _swrast_validate_point;

   if (new_state & 0x20)
      swrast->BlendFunc = _swrast_validate_blend_func;

   if (new_state & 0x40000)
      for (i = 0 ; i < ctx->Const.MaxTextureUnits ; i++)
        swrast->TextureSample[i] = _swrast_validate_texture_sample;

   if (ctx->Visual.rgbMode) {
      ;
      ;
      ;
      ;
      ;
      ;
      ;
   }
   else {
      ;
      ;
      ;
      ;
      ;
      ;
      ;
   }
}


void
_swrast_validate_derived( GLcontext *ctx )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);

   if (swrast->NewState) {
      if (swrast->NewState & (0x1000000| 0x10000| 0x20| 0x40| 0x100| 0x20000| 0x40000| 0x100000| 0x40))
        _swrast_update_rasterflags( ctx );

      if (swrast->NewState & 0x4000)
        _swrast_update_polygon( ctx );

      if (swrast->NewState & 0x200)
        _swrast_update_hint( ctx );

      if (swrast->NewState & 0x40000)
        _swrast_update_texture_env( ctx );

      swrast->NewState = 0;
      swrast->StateChanges = 0;
      swrast->InvalidateState = _swrast_invalidate_state;
   }
}


/* Public entrypoints:  See also s_accum.c, s_bitmap.c, etc.
 */
void
_swrast_Quad( GLcontext *ctx,
             const SWvertex *v0, const SWvertex *v1,
              const SWvertex *v2, const SWvertex *v3 )
{
   if (0) {
      _mesa_debug(ctx, "_swrast_Quad\n");
      _swrast_print_vertex( ctx, v0 );
      _swrast_print_vertex( ctx, v1 );
      _swrast_print_vertex( ctx, v2 );
      _swrast_print_vertex( ctx, v3 );
   }
 _mesa_debug(ctx, "_swrast_Quad Triangle call\n");

   ((SWcontext *)ctx->swrast_context)->Triangle( ctx, v0, v1, v3 );
 _mesa_debug(ctx, "_swrast_Quad Triangle call2\n");
   ((SWcontext *)ctx->swrast_context)->Triangle( ctx, v1, v2, v3 );
 _mesa_debug(ctx, "_swrast_Quad Triangle call2 return\n");

   _mesa_debug(ctx, "_swrast_Quad: v0=%p,v1=%p,v2=%p\n",v0,v1,v2);
}

//EK DEBUG
int debug_write(int *buff, int n);


void
_swrast_Triangle( GLcontext *ctx, const SWvertex *v0,
                  const SWvertex *v1, const SWvertex *v2 )
{
static int raz=0;
//   printf("-%i-\n",raz);
//EK   if (0) {
//      _mesa_debug(ctx, "_swrast_Triangle\n");
//      _swrast_print_vertex( ctx, v0 );
//      _swrast_print_vertex( ctx, v1 );
//      _swrast_print_vertex( ctx, v2 );
////   }
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
//   debug_write(tmp, 1);
//  }

// _mesa_debug(ctx, "_swrast_Triangle Triangle call\n");

   ((SWcontext *)ctx->swrast_context)->Triangle( ctx, v0, v1, v2 );

// _mesa_debug(ctx, "_swrast_Triangle Triangle call return\n");

//{
//      _mesa_debug(ctx, "_swrast_Triangle: v0=%p,v1=%p,v2=%p\n",v0,v1,v2);
//      _mesa_debug(ctx, "_swrast_Triangle: (%f,%f,%f),(%f,%f,%f),(%f,%f,%f)\n",
//        v0->win[0],v0->win[1],v0->win[2],
//        v1->win[0],v1->win[1],v1->win[2],
//        v2->win[0],v2->win[1],v2->win[2] );
//
// }
//   raz++;
//   printf("%i,%i\n",raz,v0->index);
}

void
_swrast_Line( GLcontext *ctx, const SWvertex *v0, const SWvertex *v1 )
{
   if (0) {
      _mesa_debug(ctx, "_swrast_Line\n");
      _swrast_print_vertex( ctx, v0 );
      _swrast_print_vertex( ctx, v1 );
   }
   ((SWcontext *)ctx->swrast_context)->Line( ctx, v0, v1 );
}

void
_swrast_Point( GLcontext *ctx, const SWvertex *v0 )
{
   if (0) {
      _mesa_debug(ctx, "_swrast_Point\n");
      _swrast_print_vertex( ctx, v0 );
   }
   ((SWcontext *)ctx->swrast_context)->Point( ctx, v0 );
}

void
_swrast_InvalidateState( GLcontext *ctx, GLuint new_state )
{
   if (0) {
      _mesa_debug(ctx, "_swrast_InvalidateState\n");
   }
   ((SWcontext *)ctx->swrast_context)->InvalidateState( ctx, new_state );
}

void
_swrast_ResetLineStipple( GLcontext *ctx )
{
   if (0) {
      _mesa_debug(ctx, "_swrast_ResetLineStipple\n");
   }
   ((SWcontext *)ctx->swrast_context)->StippleCounter = 0;
}

void
_swrast_allow_vertex_fog( GLcontext *ctx, GLboolean value )
{
   if (0) {
      _mesa_debug(ctx, "_swrast_allow_vertex_fog %d\n", value);
   }
   ((SWcontext *)ctx->swrast_context)->InvalidateState( ctx, 0x200 );
   ((SWcontext *)ctx->swrast_context)->AllowVertexFog = value;
}

void
_swrast_allow_pixel_fog( GLcontext *ctx, GLboolean value )
{
   if (0) {
      _mesa_debug(ctx, "_swrast_allow_pixel_fog %d\n", value);
   }
   ((SWcontext *)ctx->swrast_context)->InvalidateState( ctx, 0x200 );
   ((SWcontext *)ctx->swrast_context)->AllowPixelFog = value;
}


GLboolean
_swrast_CreateContext( GLcontext *ctx )
{
   GLuint i;
   SWcontext *swrast = (SWcontext *)_mesa_calloc(sizeof(SWcontext));

   if (0) {
      _mesa_debug(ctx, "_swrast_CreateContext\n");
   }

   if (!swrast)
      return 0x0;

   swrast->NewState = ~0;

   swrast->choose_point = _swrast_choose_point;
   swrast->choose_line = _swrast_choose_line;
   swrast->choose_triangle = _swrast_choose_triangle;

   swrast->invalidate_point = (((0x1000000| 0x10000| 0x20| 0x40| 0x100| 0x20000| 0x40000| 0x100000| 0x40) | 0x40000 | 0x200 | 0x4000 ) | 0x800000 | 0x2000 | 0x40000 | 0x400 | 0x100 | (0x400 | 0x100));
   swrast->invalidate_line = (((0x1000000| 0x10000| 0x20| 0x40| 0x100| 0x20000| 0x40000| 0x100000| 0x40) | 0x40000 | 0x200 | 0x4000 ) | 0x800000| 0x800| 0x40000| 0x400| 0x100| 0x40 | (0x400 | 0x100));
   swrast->invalidate_triangle = (((0x1000000| 0x10000| 0x20| 0x40| 0x100| 0x20000| 0x40000| 0x100000| 0x40) | 0x40000 | 0x200 | 0x4000 ) | 0x800000| 0x4000| 0x40| 0x20000| 0x20| 0x40000| (0x1000000| 0x10000| 0x20| 0x40| 0x100| 0x20000| 0x40000| 0x100000| 0x40)| 0x400| 0x100 | (0x400 | 0x100));

   swrast->Point = _swrast_validate_point;
   swrast->Line = _swrast_validate_line;
   swrast->Triangle = _swrast_validate_triangle;
   swrast->InvalidateState = _swrast_sleep;
   swrast->BlendFunc = _swrast_validate_blend_func;

   swrast->AllowVertexFog = 0x1;
   swrast->AllowPixelFog = 0x1;

   if (ctx->Visual.doubleBufferMode)
      swrast->CurrentBuffer = 0x4;
   else
      swrast->CurrentBuffer = 0x1;

   /* Optimized Accum buffer */
   swrast->_IntegerAccumMode = 0x1;
   swrast->_IntegerAccumScaler = 0.0;

   for (i = 0 ; i < 8 ; i++)
      swrast->TextureSample[i] = _swrast_validate_texture_sample;

   swrast->SpanArrays = (struct span_arrays *) _mesa_malloc(sizeof(struct span_arrays));
   if (!swrast->SpanArrays) {
      _mesa_free(swrast);
      return 0x0;
   }

   /* init point span buffer */
   swrast->PointSpan.primitive = 0x1B00;
   swrast->PointSpan.start = 0;
   swrast->PointSpan.end = 0;
   swrast->PointSpan.facing = 0;
   swrast->PointSpan.array = swrast->SpanArrays;

   ( ( ctx->Const.MaxTextureUnits > 0 ) ? ( void )0 : _assert( "ctx->Const.MaxTextureUnits > 0", "s_context.c", 535 ) );
   ( ( ctx->Const.MaxTextureUnits <= 8 ) ? ( void )0 : _assert( "ctx->Const.MaxTextureUnits <= MAX_TEXTURE_UNITS", "s_context.c", 536 ) );

   swrast->TexelBuffer = (GLchan *) _mesa_malloc(ctx->Const.MaxTextureUnits * 2048 * 4 * sizeof(GLchan));
   if (!swrast->TexelBuffer) {
      _mesa_free(swrast->SpanArrays);
      _mesa_free(swrast);
      return 0x0;
   }

   ctx->swrast_context = swrast;

   return 0x1;
}

void
_swrast_DestroyContext( GLcontext *ctx )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);

   if (0) {
      _mesa_debug(ctx, "_swrast_DestroyContext\n");
   }

   _mesa_free(swrast->SpanArrays);
   _mesa_free(swrast->TexelBuffer);
   _mesa_free(swrast);

   ctx->swrast_context = 0;
}


struct swrast_device_driver *
_swrast_GetDeviceDriverReference( GLcontext *ctx )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);
   return &swrast->Driver;
}

void
_swrast_flush( GLcontext *ctx )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);
   /* flush any pending fragments from rendering points */
   if (swrast->PointSpan.end > 0) {
      if (ctx->Visual.rgbMode) {
         if (ctx->Texture._EnabledUnits)
            _mesa_write_texture_span(ctx, &(swrast->PointSpan));
         else
            _mesa_write_rgba_span(ctx, &(swrast->PointSpan));
      }
      else {
         _mesa_write_index_span(ctx, &(swrast->PointSpan));
      }
      swrast->PointSpan.end = 0;
   }
}

void
_swrast_render_primitive( GLcontext *ctx, GLenum prim )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);
   if (swrast->Primitive == 0x0000 && prim != 0x0000) {
      _swrast_flush(ctx);
   }
   swrast->Primitive = prim;
}


void
_swrast_render_start( GLcontext *ctx )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);
   if (swrast->Driver.SpanRenderStart)
      swrast->Driver.SpanRenderStart( ctx );
   swrast->PointSpan.end = 0;
}

void
_swrast_render_finish( GLcontext *ctx )
{
   SWcontext *swrast = ((SWcontext *)ctx->swrast_context);
   if (swrast->Driver.SpanRenderFinish)
      swrast->Driver.SpanRenderFinish( ctx );

   _swrast_flush(ctx);
}



void
_swrast_print_vertex( GLcontext *ctx, const SWvertex *v )
{
   GLuint i;

   if (0) {
      _mesa_debug(ctx, "win %f %f %f %f\n",
                  v->win[0], v->win[1], v->win[2], v->win[3]);

      for (i = 0 ; i < ctx->Const.MaxTextureUnits ; i++)
        if (ctx->Texture.Unit[i]._ReallyEnabled)
           _mesa_debug(ctx, "texcoord[%d] %f %f %f %f\n", i,
                        v->texcoord[i][0], v->texcoord[i][1],
                        v->texcoord[i][2], v->texcoord[i][3]);

      _mesa_debug(ctx, "color %d %d %d %d\n",
                  v->color[0], v->color[1], v->color[2], v->color[3]);
      _mesa_debug(ctx, "spec %d %d %d %d\n",
                  v->specular[0], v->specular[1],
                  v->specular[2], v->specular[3]);
      _mesa_debug(ctx, "fog %f\n", v->fog);
      _mesa_debug(ctx, "index %d\n", v->index);
      _mesa_debug(ctx, "pointsize %f\n", v->pointSize);
      _mesa_debug(ctx, "\n");
   }
}
