if (NOT WINDOWS AND NOT APPLE)
    if (SHOULD_BUILD_FILEMANAGER_THUMBNAIL)
        add_subdirectory( thumbnail )
    endif()
endif()

add_subdirectory( kimgio )
