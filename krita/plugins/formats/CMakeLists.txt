project(kritafilters)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  add_definitions( -DCPU_32_BITS )
endif()

macro_optional_find_package(PNG)
macro_log_feature(PNG_FOUND "png" "Official PNG reference library" "http://www.libpng.org/pub/png/libpng.html" FALSE "" "Required by the Krita PNG filter")

macro_optional_find_package(OpenEXR)
macro_log_feature(OPENEXR_FOUND "OpenEXR" "High dynamic-range (HDR) image file format" "http://www.openexr.com" FALSE "" "Required by the Krita OpenEXR filter")

macro_optional_find_package(TIFF)
macro_log_feature(TIFF_FOUND "tiff" "TIFF Library and Utilities" "http://www.remotesensing.org/libtiff" FALSE "" "Required by the Krita TIFF filter")

macro_optional_find_package(JPEG)
if (JPEG_FOUND)
    DETECT_JPEG()
    if (JPEG_LIB_VERSION GREATER 70)
        set(JPEG_FOUND FALSE)
        message("Detected libpeg 8 or 9: this is NOT supported.")
    endif()
endif()
macro_log_feature(JPEG_FOUND "jpeg" "Free library for JPEG image compression. Note: libjpeg8 is NOT supported." "http://www.libjpeg-turbo.org" FALSE "" "Required by the Krita JPEG filter")

macro_optional_find_package(KF5KDcraw)
macro_log_feature( KF5KDCRAW_FOUND "KF5KDcraw" "The KDCraw library" "http://www.digikam.org/" FALSE "" "Required by the Krita RAW filter")

macro_optional_find_package(OpenJPEG)
macro_log_feature(OPENJPEG_FOUND "openjpeg" "Free library for JPEG 2000 image compression" "http://www.openjpeg.org" FALSE "" "Required by the Krita JPEG 2000 filter")

if(KF5KDCRAW_FOUND)
    add_subdirectory( raw )
endif()

if(JPEG_FOUND AND HAVE_LCMS2)
	add_subdirectory(jpeg)
endif()

if(TIFF_FOUND)
	add_subdirectory(tiff)
endif()

if(PNG_FOUND)
    add_subdirectory(png)
endif()

if(OPENEXR_FOUND)
    add_subdirectory(exr)
endif()

if(POPPLER_FOUND)
    add_subdirectory(pdf)
endif()

if(OPENJPEG_FOUND)
    add_subdirectory(jp2)
endif()

add_subdirectory(bmp)
add_subdirectory(ora)
add_subdirectory(ppm)
add_subdirectory(xcf)
add_subdirectory(psd)
add_subdirectory(odg)
add_subdirectory(qml)
add_subdirectory(tga)
add_subdirectory(heightmap)

#if(OIIO_FOUND)
#    add_subdirectory(oiio)
#endif()
