# makefile
# Created by IBM WorkFrame/2 MakeMake at 14:42:53 on 19 Dec 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Lib::Import Lib

.SUFFIXES:

.SUFFIXES: \
    .DLL .LIB .cpp .obj 

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /IGL /Iswrast /Itnl /Ss /Wpro /Tx /Fi /Si /O /Gm /Ge- /G5 /Gf /Gi /Oi /Gu /C %s

{K:\work\mesa_src\MesaDLL}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /IGL /Iswrast /Itnl /Ss /Wpro /Tx /Fi /Si /O /Gm /Ge- /G5 /Gf /Gi /Oi /Gu /C %s

.DLL.LIB:
    @echo " Lib::Import Lib "
    implib.exe %|dpfF.LIB %s

{..\MesaDLL}.DLL.LIB:
    @echo " Lib::Import Lib "
    implib.exe %|dpfF.LIB %s

all: \
    .\MesaGL2.LIB

.\MesaGL2.DLL: \
    .\vtxfmt.obj \
    .\WarpWin.obj \
    .\accum.obj \
    .\api_arrayelt.obj \
    .\api_eval.obj \
    .\api_loopback.obj \
    .\api_noop.obj \
    .\api_validate.obj \
    .\attrib.obj \
    .\a_debug.obj \
    .\blend.obj \
    .\buffers.obj \
    .\clip.obj \
    .\colortab.obj \
    .\context.obj \
    .\convolve.obj \
    .\debug.obj \
    .\depth.obj \
    .\dispatch.obj \
    .\dlist.obj \
    .\drawpix.obj \
    .\enable.obj \
    .\enums.obj \
    .\eval.obj \
    .\extensions.obj \
    .\feedback.obj \
    .\fog.obj \
    .\get.obj \
    .\glapi.obj \
    .\glthread.obj \
    .\glu.obj \
    .\glut_9x15.obj \
    .\glut_bitmap.obj \
    .\glut_cindex.obj \
    .\glut_cmap.obj \
    .\glut_event.obj \
    .\glut_gamemode.obj \
    .\glut_get.obj \
    .\glut_init.obj \
    .\glut_key.obj \
    .\glut_mesa.obj \
    .\glut_swap.obj \
    .\glut_util.obj \
    .\glut_win.obj \
    .\hash.obj \
    .\hint.obj \
    .\histogram.obj \
    .\image.obj \
    .\imports.obj \
    .\light.obj \
    .\lines.obj \
    .\matrix.obj \
    .\mmath.obj \
    .\os2_menu.obj \
    .\os2mesa.obj \
    .\os2mesaDive.obj \
    .\os2wgl.obj \
    .\os2_glx.obj \
    .\os2_winproc.obj \
    .\pixel.obj \
    .\points.obj \
    .\polygon.obj \
    .\quadric.obj \
    .\rastpos.obj \
    .\snprintf.obj \
    .\state.obj \
    .\stencil.obj \
    .\texcompress.obj \
    .\texformat.obj \
    .\teximage.obj \
    .\texobj.obj \
    .\texstate.obj \
    .\texstore.obj \
    .\texutil.obj \
    .\varray.obj \
    .\vpexec.obj \
    .\vpparse.obj \
    .\vpstate.obj \
    ..\math\libMesaMath.lib \
    ..\swrast\SwRast.lib \
    ..\swrast_setup\SwRastSetup.lib \
    ..\array_cache\libMesaAC.lib \
    ..\tnl\libMesaTnl.lib \
    {$(LIB)}MMPM2.lib \
    {$(LIB)}MesaGL2.DEF
    @echo " Link::Linker "
    icc.exe @<<
     /B" /exepack:2 /st:200000 /pmtype:pm /packd /optfunc"
     /FeMesaGL2.DLL 
     ..\math\libMesaMath.lib 
     ..\swrast\SwRast.lib 
     ..\swrast_setup\SwRastSetup.lib 
     ..\array_cache\libMesaAC.lib 
     ..\tnl\libMesaTnl.lib 
     MMPM2.lib 
     MesaGL2.DEF
     .\vtxfmt.obj
     .\WarpWin.obj
     .\accum.obj
     .\api_arrayelt.obj
     .\api_eval.obj
     .\api_loopback.obj
     .\api_noop.obj
     .\api_validate.obj
     .\attrib.obj
     .\a_debug.obj
     .\blend.obj
     .\buffers.obj
     .\clip.obj
     .\colortab.obj
     .\context.obj
     .\convolve.obj
     .\debug.obj
     .\depth.obj
     .\dispatch.obj
     .\dlist.obj
     .\drawpix.obj
     .\enable.obj
     .\enums.obj
     .\eval.obj
     .\extensions.obj
     .\feedback.obj
     .\fog.obj
     .\get.obj
     .\glapi.obj
     .\glthread.obj
     .\glu.obj
     .\glut_9x15.obj
     .\glut_bitmap.obj
     .\glut_cindex.obj
     .\glut_cmap.obj
     .\glut_event.obj
     .\glut_gamemode.obj
     .\glut_get.obj
     .\glut_init.obj
     .\glut_key.obj
     .\glut_mesa.obj
     .\glut_swap.obj
     .\glut_util.obj
     .\glut_win.obj
     .\hash.obj
     .\hint.obj
     .\histogram.obj
     .\image.obj
     .\imports.obj
     .\light.obj
     .\lines.obj
     .\matrix.obj
     .\mmath.obj
     .\os2_menu.obj
     .\os2mesa.obj
     .\os2mesaDive.obj
     .\os2wgl.obj
     .\os2_glx.obj
     .\os2_winproc.obj
     .\pixel.obj
     .\points.obj
     .\polygon.obj
     .\quadric.obj
     .\rastpos.obj
     .\snprintf.obj
     .\state.obj
     .\stencil.obj
     .\texcompress.obj
     .\texformat.obj
     .\teximage.obj
     .\texobj.obj
     .\texstate.obj
     .\texstore.obj
     .\texutil.obj
     .\varray.obj
     .\vpexec.obj
     .\vpparse.obj
     .\vpstate.obj
<<

.\vtxfmt.obj: \
    ..\MesaDLL\vtxfmt.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}api_loopback.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}vtxfmt.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}vtxfmt_tmp.h

.\vpstate.obj: \
    ..\MesaDLL\vpstate.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}hash.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}vpstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}vpexec.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}vpparse.h

.\vpparse.obj: \
    ..\MesaDLL\vpparse.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}hash.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}vpparse.h

.\vpexec.obj: \
    ..\MesaDLL\vpexec.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}vpexec.h

.\varray.obj: \
    ..\MesaDLL\varray.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}light.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dlist.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}varray.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_translate.h

.\texutil.obj: \
    ..\MesaDLL\texutil.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}image.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texformat.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texutil.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texutil_tmp.h

.\texstore.obj: \
    ..\MesaDLL\texstore.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}image.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}teximage.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}convolve.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texcompress.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texformat.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstore.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texutil.h

.\texstate.obj: \
    ..\MesaDLL\texstate.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texobj.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_xform.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_vector.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}extensions.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}teximage.h

.\texobj.obj: \
    ..\MesaDLL\texobj.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texobj.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colortab.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}hash.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}teximage.h

.\teximage.obj: \
    ..\MesaDLL\teximage.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}image.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}teximage.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}convolve.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texcompress.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texformat.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstore.h

.\texformat.obj: \
    ..\MesaDLL\texformat.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}image.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}teximage.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texformat.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texformat_tmp.h

.\texcompress.obj: \
    ..\MesaDLL\texcompress.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}image.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texcompress.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texformat.h

.\stencil.obj: \
    ..\MesaDLL\stencil.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}depth.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}stencil.h

.\state.obj: \
    ..\MesaDLL\state.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}accum.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}api_loopback.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}light.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}attrib.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}blend.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}buffers.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}depth.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}fog.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}hint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}lines.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}points.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}polygon.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}stencil.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texobj.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_xform.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_vector.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}clip.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colortab.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dlist.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}eval.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}get.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}teximage.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}varray.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}vpstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}convolve.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}feedback.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}histogram.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}pixel.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}drawpix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}rastpos.h

.\snprintf.obj: \
    ..\MesaDLL\snprintf.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h

.\rastpos.obj: \
    ..\MesaDLL\rastpos.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}light.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}simple_list.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_xform.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_vector.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}clip.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}feedback.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}rastpos.h

.\quadric.obj: \
    ..\MesaDLL\quadric.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}all.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gluP.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}nurbs.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}tess.h

.\polygon.obj: \
    ..\MesaDLL\polygon.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}polygon.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}image.h

.\points.obj: \
    ..\MesaDLL\points.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}points.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstate.h

.\pixel.obj: \
    ..\MesaDLL\pixel.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}pixel.h

.\os2_winproc.obj: \
    ..\MesaDLL\os2_winproc.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h \
    gl\os2mesa.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2mesadef.h

.\os2_glx.obj: \
    ..\MesaDLL\os2_glx.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    gl\os2mesa.h

.\os2wgl.obj: \
    ..\MesaDLL\os2wgl.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    gl\os2mesa.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2mesadef.h

.\os2mesaDive.obj: \
    ..\MesaDLL\os2mesaDive.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}depth.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h \
    gl\os2mesa.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2mesadef.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}malloc.h

.\os2mesa.obj: \
    ..\MesaDLL\os2mesa.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_xform.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_vector.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}extensions.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h \
    gl\os2mesa.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2mesadef.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colors.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texformat.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstore.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}array_cache/acache.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}swrast_setup/swrast_setup.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}swrast/s_alphabuf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}tnl/tnl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}tnl/t_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}tnl/t_pipeline.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}swrast/swrast.h

.\os2_menu.obj: \
    ..\MesaDLL\os2_winproc.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h \
    gl\os2mesa.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2mesadef.h


.\mmath.obj: \
    ..\MesaDLL\mmath.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h

.\matrix.obj: \
    ..\MesaDLL\matrix.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}buffers.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}matrix.h

.\lines.obj: \
    ..\MesaDLL\lines.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}depth.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}lines.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstate.h

.\light.obj: \
    ..\MesaDLL\light.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}light.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}simple_list.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_xform.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_vector.h

.\imports.obj: \
    ..\MesaDLL\imports.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h

.\image.obj: \
    ..\MesaDLL\image.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}image.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}histogram.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}pixel.h

.\histogram.obj: \
    ..\MesaDLL\histogram.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}image.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}histogram.h

.\hint.obj: \
    ..\MesaDLL\hint.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}hint.h

.\hash.obj: \
    ..\MesaDLL\hash.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}hash.h

.\glut_win.obj: \
    ..\MesaDLL\glut_win.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutos2.h \
    gl\os2mesa.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL_TEST.h

.\glut_util.obj: \
    ..\MesaDLL\glut_util.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h

.\glut_swap.obj: \
    ..\MesaDLL\glut_swap.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h

.\glut_mesa.obj: \
    ..\MesaDLL\glut_mesa.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h

.\glut_key.obj: \
    ..\MesaDLL\glut_key.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h

.\glut_init.obj: \
    ..\MesaDLL\glut_init.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h

.\glut_get.obj: \
    ..\MesaDLL\glut_get.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h

.\glut_gamemode.obj: \
    ..\MesaDLL\glut_gamemode.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h

.\glut_event.obj: \
    ..\MesaDLL\glut_event.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h

.\glut_cmap.obj: \
    ..\MesaDLL\glut_cmap.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}layerutil.h

.\glut_cindex.obj: \
    ..\MesaDLL\glut_cindex.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h

.\glut_bitmap.obj: \
    ..\MesaDLL\glut_bitmap.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutbitmap.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutwin32.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_x11.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}win32_glx.h

.\glut_9x15.obj: \
    ..\MesaDLL\glut_9x15.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glutbitmap.h

.\glu.obj: \
    ..\MesaDLL\glu.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}all.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gluP.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}nurbs.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}tess.h

.\glthread.obj: \
    ..\MesaDLL\glthread.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h

.\glapi.obj: \
    ..\MesaDLL\glapi.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitemp.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapioffsets.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glprocs.h

.\get.obj: \
    ..\MesaDLL\get.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}extensions.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}get.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texcompress.h

.\fog.obj: \
    ..\MesaDLL\fog.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}fog.h

.\feedback.obj: \
    ..\MesaDLL\feedback.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}feedback.h

.\extensions.obj: \
    ..\MesaDLL\extensions.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}simple_list.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}extensions.h

.\eval.obj: \
    ..\MesaDLL\eval.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}eval.h

.\enums.obj: \
    ..\MesaDLL\enums.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h

.\enable.obj: \
    ..\MesaDLL\enable.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}light.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}simple_list.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_xform.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_vector.h

.\drawpix.obj: \
    ..\MesaDLL\drawpix.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}feedback.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}drawpix.h

.\dlist.obj: \
    ..\MesaDLL\dlist.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}api_loopback.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}light.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}attrib.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}blend.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}buffers.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}depth.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}lines.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}points.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}polygon.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texobj.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_xform.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_vector.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}clip.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colortab.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}image.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dlist.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}eval.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}extensions.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}get.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}hash.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}teximage.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}varray.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}vpstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}convolve.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}feedback.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}histogram.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}pixel.h

.\dispatch.obj: \
    ..\MesaDLL\dispatch.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitemp.h

.\depth.obj: \
    ..\MesaDLL\depth.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}depth.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h

.\debug.obj: \
    ..\MesaDLL\debug.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}debug.h

.\convolve.obj: \
    ..\MesaDLL\convolve.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}image.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}convolve.h

.\context.obj: \
    ..\MesaDLL\context.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}light.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}buffers.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}fog.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}simple_list.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texobj.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_xform.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_vector.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}clip.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colortab.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dlist.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}eval.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}extensions.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}get.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}hash.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}teximage.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}varray.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}vpstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}vtxfmt.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_translate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/mathmod.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_wrapper.h

.\colortab.obj: \
    ..\MesaDLL\colortab.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colortab.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}image.h

.\clip.obj: \
    ..\MesaDLL\clip.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_xform.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_vector.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}clip.h

.\buffers.obj: \
    ..\MesaDLL\buffers.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}accum.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}buffers.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}depth.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}stencil.h

.\blend.obj: \
    ..\MesaDLL\blend.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}blend.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h

.\a_debug.obj: \
    ..\MesaDLL\a_debug.cpp

.\attrib.obj: \
    ..\MesaDLL\attrib.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}accum.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}light.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}attrib.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}blend.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}buffers.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}depth.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}enums.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}fog.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}hint.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}lines.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}points.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}polygon.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}simple_list.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}stencil.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texobj.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}texstate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_xform.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_vector.h

.\api_validate.obj: \
    ..\MesaDLL\api_validate.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}api_validate.h

.\api_noop.obj: \
    ..\MesaDLL\api_noop.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}api_noop.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}api_validate.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}light.h

.\api_loopback.obj: \
    ..\MesaDLL\api_loopback.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}colormac.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}api_loopback.h

.\api_eval.obj: \
    ..\MesaDLL\api_eval.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}api_eval.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mmath.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_eval.h

.\api_arrayelt.obj: \
    ..\MesaDLL\api_arrayelt.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}api_arrayelt.h

.\accum.obj: \
    ..\MesaDLL\accum.cpp \
    GL\os2_x11.h \
    GL\gl.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glheader.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}accum.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}imports.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}macros.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}state.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}mtypes.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}conf.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glext.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapi.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glapitable.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glthread.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}math/m_matrix.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}dd.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}Xthreads.h

.\WarpWin.obj: \
    ..\MesaDLL\WarpWin.cpp \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpWin.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}WarpGL.h \
    GL\os2_x11.h \
    GL\gl.h \
    GL\glut.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}os2_config.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}gl_mangle.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}GL/glu.h \
    {..\MesaDLL;GL;swrast;tnl;$(INCLUDE);}glu_mangle.h

.\MesaGL2.LIB: \
    .\MesaGL2.DLL
