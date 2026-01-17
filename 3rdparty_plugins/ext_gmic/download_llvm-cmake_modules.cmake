cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

set(DST_FILE "${EXT_TMP_DIR}/llvm-cmake-modules.tar.xz")
set(SHA_OK NO)
set(DOWNLOAD_TRIES 0)

while(NOT SHA_OK AND DOWNLOAD_TRIES LESS 5)
    message(STATUS "downloading file...
        ${URL}")

    file(DOWNLOAD "${URL}" "${DST_FILE}"
        STATUS DOWNLOAD_STATUS
        SHOW_PROGRESS
    )

    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_STATUS_CODE)

    if(NOT DOWNLOAD_STATUS_CODE EQUAL 0)
        message(STATUS "file download error:
        ${DOWNLOAD_STATUS}")

        math(EXPR DOWNLOAD_TRIES "${DOWNLOAD_TRIES} + 1")
        continue()
    endif()

    file(SHA256 "${DST_FILE}" ACTUAL_SHA)
    if(${URL_SHA} STREQUAL ${ACTUAL_SHA})
        set(SHA_OK YES)
    else()
        message(STATUS "file hash mismatch, retrying
          actual hash:  [${ACTUAL_SHA}]
            ")
    endif()
    math(EXPR DOWNLOAD_TRIES "${DOWNLOAD_TRIES} + 1")
endwhile()

if(NOT DOWNLOAD_STATUS_CODE EQUAL 0)
    message(FATAL_ERROR "
    ERROR: Download failed, status: ${DOWNLOAD_STATUS}
        ")
endif()

if(DOWNLOAD_TRIES GREATER_EQUAL 5)
    message(FATAL_ERROR "
    ERROR: file hash mismatch
        expected hash:  [${URL_SHA}]
          actual hash:  [${ACTUAL_SHA}]
        ")
endif()


message(STATUS "Extracting llvm-cmake Modules to ${CMAKE_CURRENT_BINARY_DIR}")
execute_process(COMMAND tar --strip-components=2 -xf "${EXT_TMP_DIR}/llvm-cmake-modules.tar.xz")
