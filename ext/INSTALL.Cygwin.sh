#!/bin/bash
export MAKEFLAGS=
export PATH=$VCROOT/VC7/Bin:$PATH
cd htk-3.3
cd HTKLib
nmake /f htk_htklib_nt.mkf all
cd ..
cd HTKTools
nmake /f htk_htktools_nt.mkf all
cd ..
cd HLMLib
nmake /f htk_hlmlib_nt.mkf all
cd ..
cd HLMTools
nmake /f htk_hlmtools_nt.mkf all
cd ..
mkdir -p /usr/local/include/htk
install bin.win32/* /usr/local/bin/
install HTKLib/*.h /usr/local/include/htk/

