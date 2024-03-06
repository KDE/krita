@echo off
setlocal enableextensions enabledelayedexpansion

set pkg_root=%~f1

echo Signing binaries in "%pkg_root%"
if not exist "%pkg_root%\" (
    echo ERROR: No packaging dir %pkg_root% 1>&2
    exit /b 1
)

if exist files-to-sign.txt (
    del /F files-to-sign.txt
)

for /r "%pkg_root%\" %%F in (*.exe *.com *.dll *.pyd) do (
    echo %%F >> files-to-sign.txt
)

python.exe -u ci-notary-service/signwindowsbinaries.py --config krita-deps-management/ci-utilities/signing/signwindowsbinaries.ini --files-from files-to-sign.txt

endlocal
