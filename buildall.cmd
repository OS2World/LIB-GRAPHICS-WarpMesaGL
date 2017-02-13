cd .\array_cache
nmake -f Makefile
cd ..\math
nmake -f Makefile
cd ..\swrast
nmake -f Makefile
cd ..\swrast_setup
nmake -f Makefile
cd  ..\tnl
nmake -f Makefile
cd ..\MesaDLL
set INCLUDE_OLD=%INCLUDE%
set INCLUDE=K:\work\mesa_src\MesaDLL;%INCLUDE_OLD%
nmake -f Makefile
set INCLUDE=%INCLUDE_OLD%
cd ..
