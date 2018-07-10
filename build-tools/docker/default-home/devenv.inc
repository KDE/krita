prepend() { [ -d "$2" ] && eval $1=\"$2\$\{$1:+':'\$$1\}\" && export $1 ; }

export KRITADIR=/home/appimage/appimage-workspace/krita-inst
export DEPSDIR=/home/appimage/appimage-workspace/deps/usr


prepend PATH $KRITADIR/bin
prepend LD_LIBRARY_PATH $KRITADIR/lib64
prepend LD_LIBRARY_PATH $KRITADIR/lib
#prepend XDG_DATA_DIRS $KRITADIR/share
prepend PKG_CONFIG_PATH $KRITADIR/lib64/pkgconfig
prepend PKG_CONFIG_PATH $KRITADIR/lib/pkgconfig

prepend PATH $DEPSDIR/bin
prepend LD_LIBRARY_PATH $DEPSDIR/lib64
prepend LD_LIBRARY_PATH $DEPSDIR/lib
#prepend XDG_DATA_DIRS $DEPSDIR/share
prepend PKG_CONFIG_PATH $DEPSDIR/lib64/pkgconfig
prepend PKG_CONFIG_PATH $DEPSDIR/lib/pkgconfig

prepend CMAKE_PREFIX_PATH $DEPSDIR

#prepend PYTHONPATH $DEPSDIR/lib/python3.5/
prepend PYTHONPATH $DEPSDIR/sip

#prepend PYQT_SIP_DIR_OVERRIDE $DEPSDIR/share/sip
