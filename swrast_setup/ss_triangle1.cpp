
#include "glheader.h"
#include "colormac.h"
//#include "macros.h"
#include "mtypes.h"

#include "tnl/t_context.h"

#include "ss_triangle.h"
//#include "ss_context.h"

#define SS_RGBA_BIT         0x1
#define SS_OFFSET_BIT      0x2
#define SS_TWOSIDE_BIT     0x4
#define SS_UNFILLED_BIT            0x8
#define SS_MAX_TRIFUNC      0x10

//static triangle_func tri_tab[SS_MAX_TRIFUNC];
//static quad_func     quad_tab[SS_MAX_TRIFUNC];

void
 _swsetup_render_line_tri( GLcontext *ctx,
                                     GLuint e0, GLuint e1, GLuint e2,
                                      GLuint facing );
 void _swsetup_render_point_tri( GLcontext *ctx,
                                      GLuint e0, GLuint e1, GLuint e2,
                                       GLuint facing );

extern void
_mesa_debug( const __GLcontext *ctx, const char *fmtString, ... );



void triangle_rgba(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
static   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
static   SWvertex *v[3];

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];

//_mesa_debug( ctx, "e0=%i,e1=%i,e2=%i\n",e0,e1,e2);
// _mesa_debug(ctx, "triangle_rgba call  _swrast_Triangle\n");

      _swrast_Triangle( ctx, v[0], v[1], v[2] );
// _mesa_debug(ctx, "triangle_rgba Return  _swrast_Triangle\n");

}

GLcontext *ctx0000;

void _Optlink triangle_rgba_Optlink(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 )
{
   struct vertex_buffer *VB = &((TNLcontext *)(ctx->swtnl_context))->vb;
static   SWvertex *verts = ((SScontext *)ctx->swsetup_context)->verts;
static   SWvertex *v[3];

   GLfloat z[3];
   GLfloat offset;
const GLenum mode = 0x1B02;
   GLuint facing = 0;

ctx0000 = ctx;

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];

//_mesa_debug( ctx, "e0=%i,e1=%i,e2=%i\n",e0,e1,e2);


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
// _mesa_debug(ctx, "triangle_rgba_Optlink call  _swrast_Triangle\n");
      _swrast_Triangle( ctx, v[0], v[1], v[2] );
// _mesa_debug(ctx, "triangle_rgba_Optlink return  _swrast_Triangle\n");
   }
}

