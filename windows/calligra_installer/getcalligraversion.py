import os
import re
# fname = 'C:/Program Files (x86)/Jenkins/jobs/cnightly25/workspace/source/libs/main/calligraversion.h'
#print(os.environ['WORKSPACE'])
#fname = os.environ['WORKSPACE'] + '/source/libs/main/calligraversion.h'
#fname = os.path.normpath(fname)
fname = 'M:/jenkins_local/calligra26/workspace/source/libs/main/calligraversion.h'
print(':: reading version from: ' + fname )
f = open(fname,'r')

versionString = []
majorVer = []
minorVer = []
releaseVer = []
success = 0

for line in f:
        if versionString == []:
                versionString = re.findall(r'\bCALLIGRA_VERSION_STRING "([A-Za-z0-9. ]*)"', line) 
        if majorVer == []:
                majorVer = re.findall(r'\bCALLIGRA_VERSION_MAJOR ([0-9]*)', line)       
        if minorVer == []:
                minorVer = re.findall(r'\bCALLIGRA_STABLE_VERSION_MINOR ([0-9]*)', line)
        if releaseVer == []:
                releaseVer = re.findall(r'\bCALLIGRA_VERSION_RELEASE ([0-9]*)', line)
        if versionString != [] and majorVer != [] and minorVer != [] and releaseVer != []:
                success = 1
                break
    
if success:
        print('set C2WINSTALL_VERSIONSTRING=' + versionString[0])
        print('set C2WINSTALL_HEADERVERSIONSEP=' + majorVer[0] + '.' + minorVer[0] + '.' + releaseVer[0])
        print('set C2WINSTALL_HEADERVERSION=' + majorVer[0] + minorVer[0] + releaseVer[0])
else:
        print(':: Error collecting version info')
