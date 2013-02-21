:: Set Calligra variables
:: 
@echo off
set KDEHOME=%USERPROFILE%\AppData\Roaming\.calligra
set KDEDIRS=%~dp0
set PATH=%~dp0bin;%~dp0lib;%~dp0lib\kde4;%PATH%
set KDESYCOCA=%~dp0sycoca

:: Launch application and all additional parameters
%*