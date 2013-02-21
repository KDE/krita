import sys
import os
import re

fname = os.getenv('WORKSPACE')
if fname:
    fname = fname + '/source/libs/main/calligraversion.h'
    os.path.normpath(fname)
else:
    fname = os.getenv('CALLIGRA_SRC')
    if fname:
        fname = fname + '/libs/main/calligraversion.h'
        os.path.normpath(fname)

if fname == None:
    print(':: Could not find Calligra sources')
    print(':: Please set CALLIGRA_SRC to point to the root of the sources,')
    print(':: or set WORKSPACE, where WORKSPACE/source/ is the root of the sources.')
    print(':: ')
    print(':: These lines are commented, to cause the installer-build to fail')
    print(':: set C2WINSTALL_VERSIONSTRING=(Unknown Version)')
    print(':: set C2WINSTALL_HEADERVERSIONSEP=0.0.0')
    print(':: set C2WINSTALL_HEADERVERSION=000')
    sys.exit()
    
print(':: reading version from: ' + fname )
f = open(fname,'r')

versionString = []
majorVer = []
minorVer = []
releaseVer = []
success = 0

for line in f:
        if versionString == []:
                versionString = re.findall(r'\bCALLIGRA_VERSION_STRING  *"([^"]*)"', line) 
                if versionString != []:
                    print(':: C2WINSTALL_VERSIONSTRING=' + versionString[0])
        if majorVer == []:
                majorVer = re.findall(r'\bCALLIGRA_VERSION_MAJOR ([0-9]*)', line)     
                if majorVer != []:
                    print(':: MAJORVER=' + majorVer[0])                
        if minorVer == []:
                minorVer = re.findall(r'\bCALLIGRA_STABLE_VERSION_MINOR ([0-9]*)', line)     
                if minorVer != []:
                    print(':: MINORVER=' + minorVer[0])    
        if releaseVer == []:
                releaseVer = re.findall(r'\bCALLIGRA_VERSION_RELEASE ([0-9]*)', line)     
                if releaseVer != []:
                    print(':: releaseVer=' + releaseVer[0])    
        if versionString != [] and majorVer != [] and minorVer != [] and releaseVer != []:
                success = 1
                break
    
if success:
        print('set C2WINSTALL_VERSIONSTRING=' + versionString[0])
        print('set C2WINSTALL_HEADERVERSIONSEP=' + majorVer[0] + '.' + minorVer[0] + '.' + releaseVer[0])
        print('set C2WINSTALL_HEADERVERSION=' + majorVer[0] + minorVer[0] + releaseVer[0])
else:
        print(':: Error collecting version info')
        print(':: These lines are commented, to cause the installer-build to fail')
        print(':: set C2WINSTALL_VERSIONSTRING=(Unknown Version)')
        print(':: set C2WINSTALL_HEADERVERSIONSEP=0.0.0')
        print(':: set C2WINSTALL_HEADERVERSION=000')
