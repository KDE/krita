set(RNGFILE ${CMAKE_SOURCE_DIR}/devtools/scripts/OpenDocument-v1.2-cs01-schema-calligra.rng)
set(RNGHEADERDIR ${CMAKE_BINARY_DIR}/libs/odf/writeodf)
if (WIN32)
    find_program(RNG2CCP_EXECUTABLE rng2cpp
        ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}
        ${CMAKE_BINARY_DIR}/bin
    )
    add_custom_command(
        OUTPUT ${RNGHEADERDIR}/writeodf.h
        COMMAND ${RNG2CCP_EXECUTABLE}
        ARGS ${RNGFILE} ${RNGHEADERDIR}
        DEPENDS rng2cpp ${RNGFILE}
        WORKING_DIRECTORY ${RNGHEADERDIR}
    )
else (WIN32)
    set(RNG2CPP_LOCATION ${CMAKE_BINARY_DIR}/devtools/rng2cpp)
    add_custom_command(
        OUTPUT ${RNGHEADERDIR}/writeodf.h
        COMMAND ${RNG2CPP_LOCATION}/rng2cpp
        ARGS ${RNGFILE} ${RNGHEADERDIR}
        DEPENDS rng2cpp ${RNGFILE}
        WORKING_DIRECTORY ${RNGHEADERDIR}
    )
endif (WIN32)
add_custom_target(writeodf.h-target DEPENDS writeodf.h)
