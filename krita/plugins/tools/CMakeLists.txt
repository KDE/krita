kde4_header()

add_subdirectory( defaulttools )
add_subdirectory( selectiontools )
add_subdirectory( tool_crop )
add_subdirectory( tool_filter )
add_subdirectory( tool_polygon )
add_subdirectory( tool_polyline )
add_subdirectory( tool_selectsimilar )
add_subdirectory( tool_star )
add_subdirectory( tool_transform )




########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = \
#    defaulttools \
#    selectiontools \
#    tool_crop \
#    tool_filter \
#    tool_polygon \
#    tool_polyline \
#    tool_selectsimilar \
#    tool_star \
#    tool_transform
