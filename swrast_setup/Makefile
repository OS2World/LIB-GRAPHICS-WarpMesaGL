# Makefile
# Created by IBM WorkFrame/2 MakeMake at 18:59:52 on 13 Dec 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Lib::Library Tool

.SUFFIXES:

.SUFFIXES: \
    .cpp .obj 

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\ /I..\X86 /I..\GL /I..\tnl /Ss /Wcmpcndcnscnvcpydcleffenuextgeninilanobsordparporprorearettrdtruunduni /Tx /O /Gm /G5 /Gf /Gi /Oi /C %s

{G:\EVGEN\MESA5\swrast_setup}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\ /I..\X86 /I..\GL /I..\tnl /Ss /Wcmpcndcnscnvcpydcleffenuextgeninilanobsordparporprorearettrdtruunduni /Tx /O /Gm /G5 /Gf /Gi /Oi /C %s

all: \
    .\SwRastSetup.lib

.\SwRastSetup.lib: \
    .\ss_triangle2.obj \
    .\ss_triangle1.obj \
    .\ss_vb.obj \
    .\ss_context.obj \
    .\ss_triangle.obj
    @echo " Lib::Library Tool "
    ilib.exe  /nol /noi SwRastSetup.lib @Makefile.rf;

.\ss_triangle2.obj: \
    ..\swrast_setup\ss_triangle2.cpp \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glheader.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}colormac.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}macros.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}imports.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}swrast/swrast.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}tnl/t_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_vector.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}ss_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}conf.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/gl.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/glext.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}config.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}mmath.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}mtypes.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_matrix.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_xform.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}swrast_setup.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}gl_mangle.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/os2_x11.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glapitable.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glthread.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}dd.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}Xthreads.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}ss_triangle.h

.\ss_triangle1.obj: \
    ..\swrast_setup\ss_triangle1.cpp \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glheader.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}colormac.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}macros.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}imports.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}swrast/swrast.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}tnl/t_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_vector.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}ss_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}conf.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/gl.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/glext.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}config.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}mmath.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}mtypes.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_matrix.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_xform.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}swrast_setup.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}gl_mangle.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/os2_x11.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glapitable.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glthread.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}dd.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}Xthreads.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}ss_triangle.h

.\ss_triangle.obj: \
    ..\swrast_setup\ss_triangle.cpp \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glheader.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}colormac.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}macros.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}imports.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}swrast/swrast.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}tnl/t_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_vector.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}ss_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}conf.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/gl.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/glext.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}config.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}mmath.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}mtypes.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_matrix.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_xform.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}swrast_setup.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}gl_mangle.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/os2_x11.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glapitable.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glthread.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}dd.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}Xthreads.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}ss_triangle.h

.\ss_context.obj: \
    ..\swrast_setup\ss_context.cpp \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glheader.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}imports.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}swrast/swrast.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}tnl/t_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_vector.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}ss_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}ss_vb.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}conf.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/gl.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/glext.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}config.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}mtypes.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_matrix.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_xform.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}swrast_setup.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}gl_mangle.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/os2_x11.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glapitable.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glthread.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}dd.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}Xthreads.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}ss_triangle.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}tnl/tnl.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}tnl/t_pipeline.h

.\ss_vb.obj: \
    ..\swrast_setup\ss_vb.cpp \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glheader.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}colormac.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}macros.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}imports.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}swrast/swrast.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}tnl/t_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_vector.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_translate.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}ss_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}ss_vb.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}conf.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/gl.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/glext.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}config.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}mmath.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glapi.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}mtypes.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_matrix.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}math/m_xform.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}swrast_setup.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}gl_mangle.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}GL/os2_x11.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glapitable.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}glthread.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}Trace/tr_context.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}dd.h \
    {..\swrast_setup;..\;..\X86;..\GL;..\tnl;$(INCLUDE);}Xthreads.h
