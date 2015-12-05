if(KF5KIO_FOUND)
    add_subdirectory( thumbnail )
endif()

add_subdirectory( kimgio )

if (APPLE)
    add_subdirectory(quicklook)
endif()
