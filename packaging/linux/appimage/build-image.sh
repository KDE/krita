# Halt on errors
set -e

# Be verbose
set -x

export BUILD_PREFIX=/home/krita/devel
export INSTALLDIR=$BUILD_PREFIX/krita.appdir/usr
export QTDIR=$BUILD_PREFIX/deps/usr
export LD_LIBRARY_PATH=$QTDIR/sip:$QTDIR/lib/x86_64-linux-gnu:$QTDIR/lib:$BUILD_PREFIX/i/lib:$LD_LIBRARY_PATH
export PATH=$QTDIR/bin:$BUILD_PREFIX/i/bin:$PATH
export PKG_CONFIG_PATH=$QTDIR/share/pkgconfig:$QTDIR/lib/pkgconfig:/usr/lib/pkgconfig:$PKG_CONFIG_PATH
export CMAKE_PREFIX_PATH=$QTDIR/lib/x86_64-linux-gnu:$BUILD_PREFIX:$QTDIR:$CMAKE_PREFIX_PATH

export PYTHONPATH=$QTDIR/sip:$QTDIR/lib/python3.5/site-packages:$QTDIR/lib/python3.5
export PYTHONHOME=$QTDIR

export APPDIR=$BUILD_PREFIX/krita.appdir
export PLUGINS=$APPDIR/usr/lib/x86_64-linux-gnu/kritaplugins/

# qjsonparser, used to add metadata to the plugins needs to work in a en_US.UTF-8 environment. That's
# not always set correctly in CentOS 6.7
export LC_ALL=en_US.UTF-8
export LANG=en_us.UTF-8

# Determine which architecture should be built
if [[ "$(arch)" = "i686" || "$(arch)" = "x86_64" ]] ; then
  ARCH=$(arch)
else
  echo "Architecture could not be determined"
  exit 1
fi

#
# Get the latest linuxdeployqt
#
cd
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" -O linuxdeployqt
chmod a+x linuxdeployqt

cp -r $QTDIR/share/kf5 $APPDIR/usr/share
cp -r $QTDIR/share/locale $APPDIR/usr/share
cp -r $QTDIR/share/mime $APPDIR/usr/share
cp -r $QTDIR/lib/python3.5 $APPDIR/usr/lib

./linuxdeployqt $APPDIR/usr/share/applications/org.kde.krita.desktop \
  -executable=$APPDIR/usr/bin/krita \
  -qmldir=$QTDIR/qml \
  -verbose=2 \
  -bundle-non-qt-libs \
  -appimage \
  -extra-plugins=$PLUGINS/kritaanimationdocker.so,$PLUGINS/kritaanimationrenderer.so,$PLUGINS/kritaarrangedocker.so,$PLUGINS/kritaartisticcolorselector.so,$PLUGINS/kritaasccdl.so,$PLUGINS/kritaassistanttool.so,$PLUGINS/kritabigbrother.so,$PLUGINS/kritablurfilter.so,$PLUGINS/kritabmpexport.so,$PLUGINS/kritabmpimport.so,$PLUGINS/kritabrushexport.so,$PLUGINS/kritabrushimport.so,$PLUGINS/kritabuginfo.so,$PLUGINS/kritachanneldocker.so,$PLUGINS/kritaclonesarray.so,$PLUGINS/kritacolorgenerator.so,$PLUGINS/kritacolorrange.so,$PLUGINS/kritacolorselectorng.so,$PLUGINS/kritacolorsfilters.so,$PLUGINS/kritacolorslider.so,$PLUGINS/kritacolorsmudgepaintop.so,$PLUGINS/kritacolorspaceconversion.so,$PLUGINS/krita_colorspaces_extensions.so,$PLUGINS/kritacompositiondocker.so,$PLUGINS/kritaconvertheighttonormalmap.so,$PLUGINS/kritaconvolutionfilters.so,$PLUGINS/kritacsvexport.so,$PLUGINS/kritacsvimport.so,$PLUGINS/kritacurvepaintop.so,$PLUGINS/kritadefaultdockers.so,$PLUGINS/kritadefaultpaintops.so,$PLUGINS/kritadefaulttools.so,$PLUGINS/kritadeformpaintop.so,$PLUGINS/kritadigitalmixer.so,$PLUGINS/kritadodgeburn.so,$PLUGINS/kritadynapaintop.so,$PLUGINS/kritaedgedetection.so,$PLUGINS/kritaembossfilter.so,$PLUGINS/kritaexample.so,$PLUGINS/kritaexperimentpaintop.so,$PLUGINS/kritaexrexport.so,$PLUGINS/kritaexrimport.so,$PLUGINS/kritaextensioncolorsfilters.so,$PLUGINS/kritafastcolortransferfilter.so,$PLUGINS/krita_filtereffects.so,$PLUGINS/kritafilterop.so,$PLUGINS/krita_flaketools.so,$PLUGINS/kritagradientmap.so,$PLUGINS/kritagriddocker.so,$PLUGINS/kritagridpaintop.so,$PLUGINS/kritahairypaintop.so,$PLUGINS/kritahalftone.so,$PLUGINS/kritahatchingpaintop.so,$PLUGINS/kritaheightmapexport.so,$PLUGINS/kritaheightmapimport.so,$PLUGINS/kritahistogramdocker.so,$PLUGINS/kritahistogram.so,$PLUGINS/kritahistorydocker.so,$PLUGINS/kritaimagedocker.so,$PLUGINS/kritaimageenhancement.so,$PLUGINS/kritaimagesize.so,$PLUGINS/kritaimagesplit.so,$PLUGINS/kritaindexcolors.so,$PLUGINS/kritajpegexport.so,$PLUGINS/kritajpegimport.so,$PLUGINS/krita_karbontools.so,$PLUGINS/kritakraexport.so,$PLUGINS/kritakraimport.so,$PLUGINS/kritalayergroupswitcher.so,$PLUGINS/kritalayersplit.so,$PLUGINS/kritalcmsengine.so,$PLUGINS/kritalevelfilter.so,$PLUGINS/kritalutdocker.so,$PLUGINS/kritametadataeditor.so,$PLUGINS/kritamodifyselection.so,$PLUGINS/kritanoisefilter.so,$PLUGINS/kritanormalize.so,$PLUGINS/kritaoffsetimage.so,$PLUGINS/kritaoilpaintfilter.so,$PLUGINS/kritaoraexport.so,$PLUGINS/kritaoraimport.so,$PLUGINS/kritaoverviewdocker.so,$PLUGINS/kritapalettedocker.so,$PLUGINS/kritaparticlepaintop.so,$PLUGINS/kritapatterndocker.so,$PLUGINS/kritapatterngenerator.so,$PLUGINS/kritapdfimport.so,$PLUGINS/kritaphongbumpmap.so,$PLUGINS/kritapixelizefilter.so,$PLUGINS/kritapngexport.so,$PLUGINS/kritapngimport.so,$PLUGINS/kritaposterize.so,$PLUGINS/kritappmexport.so,$PLUGINS/kritappmimport.so,$PLUGINS/kritapresetdocker.so,$PLUGINS/kritapresethistory.so,$PLUGINS/kritapsdexport.so,$PLUGINS/kritapsdimport.so,$PLUGINS/kritapykrita.so,$PLUGINS/kritaqmic.so,$PLUGINS/kritaqmlexport.so,$PLUGINS/kritaraindropsfilter.so,$PLUGINS/kritarandompickfilter.so,$PLUGINS/krita_raw_import.so,$PLUGINS/kritaresourcemanager.so,$PLUGINS/kritarotateimage.so,$PLUGINS/kritaroundcornersfilter.so,$PLUGINS/kritaroundmarkerpaintop.so,$PLUGINS/kritaselectiontools.so,$PLUGINS/kritaseparatechannels.so,$PLUGINS/krita_shape_artistictext.so,$PLUGINS/krita_shape_image.so,$PLUGINS/krita_shape_paths.so,$PLUGINS/krita_shape_text.so,$PLUGINS/kritashearimage.so,$PLUGINS/kritasketchpaintop.so,$PLUGINS/kritasmallcolorselector.so,$PLUGINS/kritasmalltilesfilter.so,$PLUGINS/kritaspecificcolorselector.so,$PLUGINS/kritaspraypaintop.so,$PLUGINS/kritaspriterexport.so,$PLUGINS/kritasvgcollectiondocker.so,$PLUGINS/kritasvgimport.so,$PLUGINS/kritatangentnormalpaintop.so,$PLUGINS/kritatasksetdocker.so,$PLUGINS/kritatgaexport.so,$PLUGINS/kritatgaimport.so,$PLUGINS/kritathreshold.so,$PLUGINS/kritatiffexport.so,$PLUGINS/kritatiffimport.so,$PLUGINS/krita_tool_basicflakes.so,$PLUGINS/kritatoolcrop.so,$PLUGINS/kritatooldyna.so,$PLUGINS/kritatoollazybrush.so,$PLUGINS/kritatoolpolygon.so,$PLUGINS/kritatoolpolyline.so,$PLUGINS/kritatoolSmartPatch.so,$PLUGINS/krita_tool_svgtext.so,$PLUGINS/kritatooltransform.so,$PLUGINS/kritatouchdocker.so,$PLUGINS/kritaunsharpfilter.so,$PLUGINS/kritavideoexport.so,$PLUGINS/kritawavefilter.so,$PLUGINS/kritawaveletdecompose.so,$PLUGINS/kritaxcfimport.so,$APPDIR/usr/lib/x86_64-linux-gnu/qml/org/krita/sketch/libkritasketchplugin.so,$QTDIR/share/locale,$APPDIR/usr/lib/x86_64-linux-gnu/qml/org/krita/draganddrop/libdraganddrop.so

