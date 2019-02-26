project(kritafilters)

add_subdirectory(libkra)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  add_definitions( -DCPU_32_BITS )
endif()

if(JPEG_FOUND AND HAVE_LCMS2)
	add_subdirectory(jpeg)
endif()

if(TIFF_FOUND)
	add_subdirectory(tiff)
endif()

if(PNG_FOUND)
    add_subdirectory(png)
    add_subdirectory(csv)
endif()

if(OPENEXR_FOUND)
    add_subdirectory(exr)
endif()

if(Poppler_Qt5_FOUND)
    add_subdirectory(pdf)
endif()

if(LIBRAW_FOUND)
    add_subdirectory(raw)
endif()

add_subdirectory(svg)
add_subdirectory(qimageio)
add_subdirectory(ora)
add_subdirectory(ppm)
add_subdirectory(xcf)
add_subdirectory(psd)
add_subdirectory(qml)
add_subdirectory(tga)
add_subdirectory(heightmap)
add_subdirectory(brush)
add_subdirectory(spriter)
add_subdirectory(kra)

if (GIF_FOUND)
    add_subdirectory(gif)
endif()

if (HEIF_FOUND)
    add_subdirectory(heif)
endif()
