
#include "glheader.h"
#include "colormac.h"
#include "mtypes.h"

#include "tnl/t_context.h"

#include "ss_triangle.h"


extern void triangle_rgba(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 );

//extern void _Optlink triangle_rgba_Optlink(GLcontext *ctx, GLuint e0, GLuint e1, GLuint e2 );

void _Optlink quadfunc_rgba( GLcontext *ctx, GLuint v0,
                      GLuint v1, GLuint v2, GLuint v3 )
{

//_mesa_debug( ctx, "quadfunc_rgba e0=%i,e1=%i,e2=%i,e3=%i\n",v0,v1,v2,v3);

      triangle_rgba( ctx, v0, v1, v3 );
      triangle_rgba( ctx, v1, v2, v3 );
//      triangle_rgba_Optlink( ctx, v0, v1, v3 );
//      triangle_rgba_Optlink( ctx, v1, v2, v3 );
}

