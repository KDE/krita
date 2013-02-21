::  package_calligra.bat
::
::  Copies relevant files from the calligra inst and kderoot folders 
::
@echo off

::  Safety check:
::  Make sure key variables are defined

IF "%C2WINSTALL_INPUT%"=="" (
	echo !!! C2WINSTALL VARIABLES NOT PROPERLY DEFINED !!!
	echo Operation cancelled.
	goto :eof
	pause
)
IF "%C2WINSTALL_OUTPUT%"=="" (
	echo !!! C2WINSTALL VARIABLES NOT PROPERLY DEFINED !!!
	echo Operation cancelled.
	goto :eof
	pause
)

::  Create redistribution directories
rm -rf %C2WINSTALL_INPUT%
mkdir %C2WINSTALL_INPUT%
mkdir %C2WINSTALL_OUTPUT%

mkdir %C2WINSTALL_INPUT%\bin
mkdir %C2WINSTALL_INPUT%\etc
mkdir %C2WINSTALL_INPUT%\lib
mkdir %C2WINSTALL_INPUT%\lib\kde4
mkdir %C2WINSTALL_INPUT%\plugins
mkdir %C2WINSTALL_INPUT%\share

echo Copying plugins
cp %CALLIGRA_INST%\lib\kde4/artistictextshape.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/autocorrect.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/basicflakesplugin.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/calligradocinfopropspage.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/calligradockers.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/calligraimagethumbnail.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/calligrathumbnail.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/changecase.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/comicbookthumbnail.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/defaulttools.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/htmlthumbnail.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/imagethumbnail.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/jpegthumbnail.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/karbon_flattenpathplugin.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/karbon_refinepathplugin.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/karbon_roundcornersplugin.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/karbon_whirlpinchplugin.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/karbon1ximport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/karbonfiltereffects.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/karbonimageexport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/karbonpart.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/karbonsvgexport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/karbonsvgimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/karbontools.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/karbonxfigimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kded*.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kio*.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kfile*.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kolcmsengine.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/krita_colorspaces_extensions.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/krita_raw_import.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaartisticcolorselector.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritabigbrother.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritablurfilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritabmpexport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritabmpimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritachalkpaintop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritachanneldocker.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritacolorgenerator.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritacolorrange.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritacolorselectorng.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritacolorsfilters.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritacolorsmudgepaintop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritacolorspaceconversion.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritacompositiondocker.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaconvolutionfilters.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritacurvepaintop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritadefaultdockers.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritadefaultpaintops.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritadefaulttools.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritadeformpaintop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritadigitalmixer.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritadodgeburn.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritadropshadow.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritadynapaintop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaembossfilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaexample.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaexperimentpaintop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaextensioncolorsfilters.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritafastcolortransferfilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritafilterop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaflipbookdocker.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaflipbookimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritagridpaintop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritahairypaintop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritahatchingpaintop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritahistogram.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritahistorydocker.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaimagedocker.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaimageenhancement.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaimagesize.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaimagesplit.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritajp2export.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritajp2import.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritajpegexport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritajpegimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritalayercompose.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritalevelfilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritametadataeditor.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritamodifyselection.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritanoisefilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaodgimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaoilpaintfilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaoraexport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaoraimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritapart.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaparticlepaintop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritapatterndocker.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritapdfimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaphongbumpmap.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritapixelizefilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritapngexport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritapngimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritappmexport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritappmimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritapresetdocker.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritapsdexport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritapsdimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaqmlexport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaraindropsfilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritarandompickfilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritarotateimage.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaroundcornersfilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritarulerassistanttool.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaselectiontools.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaseparatechannels.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritashearimage.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritasketchpaintop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritasmallcolorselector.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritasmalltilesfilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritasobelfilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaspecificcolorselector.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaspraypaintop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritatasksetdocker.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritatiffexport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritatiffimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritatoolcrop.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritatooldyna.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritatoolgrid.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritatoolperspectivegrid.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritatoolpolygon.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritatoolpolyline.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritatooltext.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritatooltransform.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaunsharpfilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritawavefilter.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kritaxcfimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/kstyle_oxygen_config.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/pathshapes.dll %C2WINSTALL_INPUT%\lib\kde4

mkdir %C2WINSTALL_INPUT%\lib\kde4\styles
cp %CALLIGRA_INST%\lib\kde4/plugins\styles\oxygen.dll %C2WINSTALL_INPUT%\lib\kde4\styles

mkdir %C2WINSTALL_INPUT%\lib\kde4\styles\imageformats
cp %CALLIGRA_INST%\lib\kde4/plugins\imageformats\kimg_dds.dll %C2WINSTALL_INPUT%\lib\kde4\imageformats
cp %CALLIGRA_INST%\lib\kde4/plugins\imageformats\kimg_pcx.dll %C2WINSTALL_INPUT%\lib\kde4\imageformats
cp %CALLIGRA_INST%\lib\kde4/plugins\imageformats\kimg_ras.dll %C2WINSTALL_INPUT%\lib\kde4\imageformats
cp %CALLIGRA_INST%\lib\kde4/plugins\imageformats\kimg_xcf.dll %C2WINSTALL_INPUT%\lib\kde4\imageformats
cp %CALLIGRA_INST%\lib\kde4/plugins\imageformats\kimg_eps.dll %C2WINSTALL_INPUT%\lib\kde4\imageformats
cp %CALLIGRA_INST%\lib\kde4/plugins\imageformats\kimg_pic.dll %C2WINSTALL_INPUT%\lib\kde4\imageformats
cp %CALLIGRA_INST%\lib\kde4/plugins\imageformats\kimg_rgb.dll %C2WINSTALL_INPUT%\lib\kde4\imageformats
cp %CALLIGRA_INST%\lib\kde4/plugins\imageformats\kimg_xview.dll %C2WINSTALL_INPUT%\lib\kde4\imageformats
cp %CALLIGRA_INST%\lib\kde4/plugins\imageformats\kimg_jp2.dll %C2WINSTALL_INPUT%\lib\kde4\imageformats
cp %CALLIGRA_INST%\lib\kde4/plugins\imageformats\kimg_psd.dll %C2WINSTALL_INPUT%\lib\kde4\imageformats
cp %CALLIGRA_INST%\lib\kde4/plugins\imageformats\kimg_tga.dll %C2WINSTALL_INPUT%\lib\kde4\imageformats

cp %CALLIGRA_INST%\lib\kde4/kurisearchfilter.dll  %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/localdomainurifilter.dll  %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/svgthumbnail.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/textshape.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/textthumbnail.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/windowsexethumbnail.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/windowsimagethumbnail.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/wmfexport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/wmfimport.dll %C2WINSTALL_INPUT%\lib\kde4
cp %CALLIGRA_INST%\lib\kde4/wpgimport.dll %C2WINSTALL_INPUT%\lib\kde4

echo Copying Libraries

cp %CALLIGRA_INST%\bin\*dll  %C2WINSTALL_INPUT%\bin\

echo Copying Exes

cp %CALLIGRA_INST%\bin\krita.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\calligraconverter.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kwalletd.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\dbus-daemon.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\dbus-launch.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kbuildsycoca4.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kconf_update.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kconfig_compiler.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kde4-config.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kdebugdialog.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kded4.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kdeinit4.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kioexec.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kioclient.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kioslave.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kquitapp.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\ktraderclient.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kwalletd.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\kwrapper4.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\klauncher.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\knotify4.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\drkonqi.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\update-mime-database.exe  %C2WINSTALL_INPUT%\bin\
cp %CALLIGRA_INST%\bin\khelpcenter.exe  %C2WINSTALL_INPUT%\bin\

echo Copying /etc directory for dbus

cp -r %CALLIGRA_INST%\etc\dbus-1  %C2WINSTALL_INPUT%\etc\

echo Copying Qt plugins

cp -r %CALLIGRA_INST%\plugins\imageformats %C2WINSTALL_INPUT%\plugins
cp -r %CALLIGRA_INST%\plugins\iconengines %C2WINSTALL_INPUT%\plugins

echo Copying the /share/applications directory
mkdir  %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\calligra.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_bmp.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_flipbook.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_jp2.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_jpeg.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_odg.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_ora.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_pdf.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_png.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_ppm.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_psd.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_qml.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_raw.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_tiff.desktop %C2WINSTALL_INPUT%\share\applications\kde4
cp %CALLIGRA_INST%\share\applications\kde4\krita_xcf.desktop %C2WINSTALL_INPUT%\share\applications\kde4

echo Copying the /share/apps directory

mkdir %C2WINSTALL_INPUT%\share\applications\kde4\apps

cp -r %CALLIGRA_INST%\share\apps\calligra %C2WINSTALL_INPUT%\share\apps
cp -r %CALLIGRA_INST%\share\apps\color-schemes %C2WINSTALL_INPUT%\share\apps
cp -r %CALLIGRA_INST%\share\apps\desktoptheme %C2WINSTALL_INPUT%\share\apps
cp -r %CALLIGRA_INST%\share\apps\hardwarenotifications %C2WINSTALL_INPUT%\share\apps
cp -r %CALLIGRA_INST%\share\apps\kauth %C2WINSTALL_INPUT%\share\apps
cp -r %CALLIGRA_INST%\share\apps\kde %C2WINSTALL_INPUT%\share\apps\
cp -r %CALLIGRA_INST%\share\apps\kdeui %C2WINSTALL_INPUT%\share\apps\
cp -r %CALLIGRA_INST%\share\apps\kdewidgets %C2WINSTALL_INPUT%\share\apps\
cp -r %CALLIGRA_INST%\share\apps\kio* %C2WINSTALL_INPUT%\share\apps\
cp -r %CALLIGRA_INST%\share\apps\knewstuff %C2WINSTALL_INPUT%\share\apps\
cp -r %CALLIGRA_INST%\share\apps\kritaplugins %C2WINSTALL_INPUT%\share\apps\
cp -r %CALLIGRA_INST%\share\apps\krita %C2WINSTALL_INPUT%\share\apps\
cp -r %CALLIGRA_INST%\share\apps\kssl %C2WINSTALL_INPUT%\share\apps\
cp -r %CALLIGRA_INST%\share\apps\kstyle %C2WINSTALL_INPUT%\share\apps\
cp -r %CALLIGRA_INST%\share\apps\kwalletd %C2WINSTALL_INPUT%\share\apps\
cp -r %CALLIGRA_INST%\share\apps\mime %C2WINSTALL_INPUT%\share\apps\

echo Copying the /share/color directory
cp -r %CALLIGRA_INST%\share\color %C2WINSTALL_INPUT%\share\

echo Copying the /share/config directory
mkdir  %C2WINSTALL_INPUT%\share\config
cp -r %CALLIGRA_INST%\share\config\colors %C2WINSTALL_INPUT%\share\config
cp -r %CALLIGRA_INST%\share\config\ui %C2WINSTALL_INPUT%\share\config
cp -r %CALLIGRA_INST%\share\config\krita* %C2WINSTALL_INPUT%\share\config
cp -r %CALLIGRA_INST%\share\config\kdebug* %C2WINSTALL_INPUT%\share\config
cp -r %CALLIGRA_INST%\share\config\khot* %C2WINSTALL_INPUT%\share\config
cp -r %CALLIGRA_INST%\share\config\kdebugrc %C2WINSTALL_INPUT%\share\config
cp -r %CALLIGRA_INST%\share\config\kdeglobals %C2WINSTALL_INPUT%\share\config

echo Copying the /share/dbus-1 directory
mkdir  %C2WINSTALL_INPUT%\share\dbus-1\interfaces
mkdir  %C2WINSTALL_INPUT%\share\dbus-1\services
mkdir  %C2WINSTALL_INPUT%\share\dbus-1\system-services

cp -r %CALLIGRA_INST%\share\dbus-1\interfaces\org.kde.* %C2WINSTALL_INPUT%\share\dbus-1\interfaces
cp -r %CALLIGRA_INST%\share\dbus-1\services\org.kde.* %C2WINSTALL_INPUT%\share\dbus-1\services

echo Copying the /share/icons directory (need to be cleaned out!)
cp -r %CALLIGRA_INST%\share\icons %C2WINSTALL_INPUT%\share\

echo Copying the /share/mime directory 
cp -r %CALLIGRA_INST%\share\mime %C2WINSTALL_INPUT%\share\

echo Copying the /share/kde4 directory
mkdir  %C2WINSTALL_INPUT%\share\kde4\services

cp %CALLIGRA_INST%\share\kde4\services\*.protocol %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\krita*.desktop %C2WINSTALL_INPUT%\share\kde4\services

cp %CALLIGRA_INST%\share\kde4\services\artistictextshape.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\autocorrect.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\basicflakesplugin.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\calligra_odg_thumbnail.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\calligradocinfopropspage.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\calligradockers.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\changecase.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\comicbookthumbnail.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\componentchooser.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\defaulttools.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\desktopthumbnail.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\directorythumbnail.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\djvuthumbnail.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\htmlthumbnail.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\imagethumbnail.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\jpegthumbnail.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\kolcmsengine.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\kwalletd.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\pathshapes.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\textshape.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\textthumbnail.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\thumbnail.protocol %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\filetypes.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\fixhosturifilter.desktop  %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\htmlthumbnail.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\icons.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\karbonfiltereffects.desktop %C2WINSTALL_INPUT%\share\kde4\service
cp %CALLIGRA_INST%\share\kde4\services\karbontools.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\kfilemodule.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\kglobalaccel.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\knotify4.desktop  %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\kshorturifilter.desktop  %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\kuiserver.desktop  %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\kuriikwsfilter.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\kurisearchfilter.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\localdomainurifilter.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\platform.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\svgthumbnail.desktop %C2WINSTALL_INPUT%\share\kde4\services
cp %CALLIGRA_INST%\share\kde4\services\windowsimagethumbnail.desktop %C2WINSTALL_INPUT%\share\kde4\services

cp -r %CALLIGRA_INST%\share\kde4\services\qimageioplugins %C2WINSTALL_INPUT%\share\kde4\services

mkdir  %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\application.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\calligra*.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\filtereffect.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\flake.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\flakedevice.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\flakeshape.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\flaketool.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\inlinetextobject.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\karbon_module.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\kconfigbackend.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\kdedmodule.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\kfileitemactionplugin.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\kfileplugin.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\kfilewrite.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\kiofilemodule.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\knotifynotifymethod.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\kofilter.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\kofilterwrapper.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\koplugin.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\kpart.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\kplugininfo.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\krita_brush.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\krita_dock.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\krita_filter.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\krita_generator.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\krita_paintop.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\krita_plugin.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\krita_tool.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\kurifilterplugin.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\pigment.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\pigmentextension.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\qimageio_plugin.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\texteditingplugin.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\textvariableplugin.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes
cp %CALLIGRA_INST%\share\kde4\servicetypes\thumbcreator.desktop %C2WINSTALL_INPUT%\share\kde4\servicetypes

cp %C2WINSTALL_INPUT%\..\c2winstaller\res\package\env.bat %C2WINSTALL_INPUT%
