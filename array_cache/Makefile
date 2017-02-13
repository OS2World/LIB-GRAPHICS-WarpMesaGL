# Makefile
# Created by IBM WorkFrame/2 MakeMake at 15:00:42 on 18 Dec 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Lib::Library Tool

.SUFFIXES:

.SUFFIXES: \
    .cpp .obj 

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\ /I..\X86 /I..\GL /Ss /Wlanobsordprorearettru /Tx /Os- /O /Gm /G5 /Gi /C %s

{G:\EVGEN\MESA5\array_cache}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\ /I..\X86 /I..\GL /Ss /Wlanobsordprorearettru /Tx /Os- /O /Gm /G5 /Gi /C %s

all: \
    .\libMesaAC.lib

.\libMesaAC.lib: \
    .\ac_context.obj \
    .\ac_import.obj
    @echo " Lib::Library Tool "
    ilib.exe  /noi libMesaAC.lib @Makefile.rf;

.\ac_context.obj: \
    ..\array_cache\ac_context.cpp \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}glheader.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}macros.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}imports.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}mmath.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}mtypes.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}array_cache/ac_context.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}conf.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}GL/os2_config.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}GL/gl.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}GL/glext.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}config.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}glapitable.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}glthread.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}math/m_matrix.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}Trace/tr_context.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}dd.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}array_cache/acache.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}gl_mangle.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}GL/os2_x11.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}Xthreads.h

.\ac_import.obj: \
    ..\array_cache\ac_import.cpp \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}glheader.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}macros.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}imports.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}mmath.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}mtypes.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}math/m_translate.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}array_cache/ac_context.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}conf.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}GL/os2_config.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}GL/gl.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}GL/glext.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}config.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}glapitable.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}glthread.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}math/m_matrix.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}Trace/tr_context.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}dd.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}array_cache/acache.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}gl_mangle.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}GL/os2_x11.h \
    {..\array_cache;..\;..\X86;..\GL;$(INCLUDE);}Xthreads.h
