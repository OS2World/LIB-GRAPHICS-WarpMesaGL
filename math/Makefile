# Makefile
# Created by IBM WorkFrame/2 MakeMake at 18:57:39 on 13 Dec 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Lib::Library Tool

.SUFFIXES:

.SUFFIXES: \
    .cpp .obj 

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\ /I..\X86 /I..\GL /Ss /Wlanobsordprorearettruund /Ti /O /Gm /G5 /Tm /Gf /Gi /Oi /C %s

{..\math}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\ /I..\X86 /I..\GL /Ss /Wlanobsordprorearettruund /Ti /O /Gm /G5 /Tm /Gf /Gi /Oi /C %s

all: \
    .\libMesaMath.lib

.\libMesaMath.lib: \
    .\m_vector.obj \
    .\m_translate.obj \
    .\m_xform.obj \
    .\m_eval.obj \
    .\m_matrix.obj
    @echo " Lib::Library Tool "
    ilib.exe  /nol /noi libMesaMath.lib @Makefile.rf;

.\m_vector.obj: \
    ..\math\m_vector.cpp \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}glheader.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}macros.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_matrix.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}conf.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/gl.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/glext.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}imports.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}config.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}mtypes.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}math/m_vector.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}gl_mangle.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/os2_x11.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}glapitable.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}glthread.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}Trace/tr_context.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}dd.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}Xthreads.h

.\m_translate.obj: \
    ..\math\m_translate.cpp \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}glheader.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}macros.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}mmath.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_matrix.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_translate.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}conf.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/gl.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/glext.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}imports.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}config.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}mtypes.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}gl_mangle.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/os2_x11.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}glapitable.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}glthread.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}Trace/tr_context.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}dd.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}Xthreads.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}colormac.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_trans_tmp.h

.\m_matrix.obj: \
    ..\math\m_matrix.cpp \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}glheader.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}macros.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}mmath.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_matrix.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}conf.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/gl.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/glext.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}imports.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}gl_mangle.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/os2_x11.h

.\m_eval.obj: \
    ..\math\m_eval.cpp \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}glheader.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_eval.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}conf.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/gl.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/glext.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}config.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}gl_mangle.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/os2_x11.h

.\m_xform.obj: \
    ..\math\m_xform.cpp \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}glheader.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}macros.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}mmath.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_eval.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_matrix.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_translate.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_xform.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}mathmod.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_debug.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}X86/common_x86_asm.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_xform_tmp.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_clip_tmp.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_norm_tmp.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_dotprod_tmp.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}m_copy_tmp.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}conf.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/gl.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/glext.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}imports.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}config.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}mtypes.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}math/m_vector.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}common_x86_features.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}x86.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}3dnow.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}sse.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}gl_mangle.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}GL/os2_x11.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}glapitable.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}glthread.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}Trace/tr_context.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}dd.h \
    {..\math;..\;..\X86;..\GL;$(INCLUDE);}Xthreads.h
