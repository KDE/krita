= Notes for Packagers =

== Patching Qt ==

Qt 5.6 is currently the recommended version to build Krita with on all platforms. However, Qt 5.6 on Linux needs to be patched for https://bugreports.qt.io/browse/QTBUG-44964 .

The patch in 3rdparty/ext_qt/qt-no-motion-compression.diff 


== Package Contents ==

We recommend that all of Krita packaged in one package: there is no need to split Krita up. In particular, do not make a separate package out of the plugins directory; without the plugins Krita will not even start.

Krita does not install header files, so there is no need for a corresponding -dev(el) package.

== Third Party Libraries ==

The top-level 3rd-party directory is not relevant for packaging: it only contains CMake projects for all of Krita's dependencies which are used for building Krita on Windows and OSX. It is not called from the top-level CMakeLists.txt project.

There are four forks of 3rd party libraries that are relevant and cannot be replaced by system libraries:

* plugins/impex/raw/3rdparty contains a fork of kdcraw. Upstream removed most functionality from this library and is in general unable to provide a stable API. The library has been renamed to avoid conflicts with upstream kdcraw.

* plugins/impex/xcf/3rdparty contains the xcftools code. This has never been released as a library

* libs/image/3rdparty contains einspline. This code is directly linked into the kritaimage library and has never been released as a separate library.

== Build flags ==

Krita no longer supports a build without OpenGL.

For alpha and beta packages, please build with debug output enabled, but for production packages the -DCMAKE_CXX_FLAGS="-DKDE_NO_DEBUG_OUTPUT" is recommended. A significant performance increase will be the result.

If you build Krita with RelWithDebInfo to be able to create a corresponding  -dbg package, please define -DQT_NO_DEBUG=1 as well to disable asserts. 

== Dependencies ==

Krita depends on:

  * boost and the boost-system library        
  * eigen3       
  * exiv2   
  * fftw3   
  * gsl       
  * ilmbase   
  * jpeg: Note that libjpeg-turbo is recommended.       
  * lcms2     
  * libraw    
  * opencolorio
  * openexr   
  * png        
  * poppler-qt5
  * pthreads   
  * qt-5: Note that Qt 5.6 is _strongly_ recommended. Qt 5.5 has bugs that interfere with proper handling of tablet events
  * tiff  
  * vc: this is a build-time dependency only
  * zlib

And the following KDE Frameworks:

 * Archive 
 * Completion
 * Config 
 * CoreAddons
 * GuiAddons 
 * I18n 
 * ItemModels 
 * ItemViews
 * KCrash
 * WidgetsAddons 
 * WindowSystem
