include_directories(${CMAKE_SOURCE_DIR}/krita/ui/obsolete/)

#add_subdirectory( bumpmap )
MESSAGE("Krita: port the bumpmap filter")

#add_subdirectory( cimg )
MESSAGE("Krita: port cimg and fix it")

add_subdirectory( convolutionfilters )
add_subdirectory( embossfilter )
add_subdirectory( example )
add_subdirectory( imageenhancement )

message("Krita: port the oilpaint filter")
# add_subdirectory( oilpaintfilter )

message("Krita: port the pixelize filter")
# add_subdirectory( pixelizefilter )

message("Krita: port the raindrops filter")
# add_subdirectory( raindropsfilter )

message("Krita: port the round corners filter")
# add_subdirectory( roundcorners )

message("Krita: port the small tiles filter")
# add_subdirectory( smalltilesfilter )

message("Krita: port the cubism filter")
# add_subdirectory( cubismfilter )

add_subdirectory( sobelfilter )
add_subdirectory( colorsfilters )
add_subdirectory( wavefilter )
add_subdirectory( noisefilter )
add_subdirectory( lenscorrectionfilter )
add_subdirectory( randompickfilter )
add_subdirectory( blur )
add_subdirectory( fastcolortransfer )
add_subdirectory( colors )
add_subdirectory( unsharp )
