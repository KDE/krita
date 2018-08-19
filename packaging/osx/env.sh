export PATH=/Applications/CMake.app/Contents/bin/:/Users/boud/bin:/Users/boud/dev/deps/bin:/Users/boud/dev/deps/lib:/Users/boud/dev/i/bin:/Users/boud/i/lib:$PATH
export LIBRARY_PATH=/Users/boud/dev/deps/lib:/Users/boud/dev/i/lib:$LIBRARY_PATH
export C_INCLUDE_PATH=/Users/boud/dev/deps/include:/Users/boud/dev/i/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=/Users/boud/dev/deps/include:/Users/boud/dev/i/include:$CPLUS_INCLUDE_PATH
#export KRITA_PLUGIN_PATH=/Users/boud/dev/i/
export PKG_CONFIG_PATH=/Users/boud/dev/deps/lib/pkgconfig:/Users/boud/dev/deps/share/pkgconfig:$PKG_CONFIG_PATH
 
alias ls='ls -AFG'
 
###
## A script to setup some needed variables and functions for KDE 4 development.
 
prepend() { [ -d "$2" ] && eval $1=\"$2\$\{$1:+':'\$$1\}\" && export $1 ; }
 
# This will make the debug output prettier
export KDE_COLOR_DEBUG=1
export QTEST_COLORED=1

# We only support from 10.11 up
export QMAKE_MACOSX_DEPLOYMENT_TARGET=10.11
export MACOSX_DEPLOYMENT_TARGET=10.11
export SDKROOT=/Applications/Xcode.app/Contents/developer/Platforms/MacOSX.platform/developer/SDKs/MacOSX10.11.sdk

# Python
export DEPS_INSTALL_PREFIX=/Users/boud/dev/deps
export PYTHONPATH=$DEPS_INSTALL_PREFIX/sip:$DEPS_INSTALL_PREFIX/lib/python3.5/site-packages:$DEPS_INSTALL_PREFIX/lib/python3.5
export PYTHONHOME=$DEPS_INSTALL_PREFIX
