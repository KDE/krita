:: Set Calligra variables
:: 
@echo off
set KDEHOME=%appdata%\krita
set KDESYCOCA=%~dp0sycoca
set XDG_DATA_DIRS=%~dp0share
set KDEDIRS=%~dp0
set KDEDIR=%~dp0
set PATH=%~dp0bin;%~dp0lib;%~dp0lib\krita;%PATH%
set QT_PLUGIN_PATH=%~dp0lib
:: Launch application and all additional parameters
%*
