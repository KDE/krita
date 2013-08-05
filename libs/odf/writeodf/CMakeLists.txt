set(RNGFILE ${CMAKE_SOURCE_DIR}/devtools/scripts/OpenDocument-v1.2-cs01-schema-calligra.rng)
set(RNGHEADERDIR ${CMAKE_BINARY_DIR}/libs/odf/writeodf)
add_custom_command(
    OUTPUT ${RNGHEADERDIR}/writeodf.h
    COMMAND ${CMAKE_BINARY_DIR}/devtools/rng2cpp/rng2cpp
    ARGS ${RNGFILE} ${RNGHEADERDIR}
    DEPENDS rng2cpp ${RNGFILE}
    WORKING_DIRECTORY ${RNGHEADERDIR}
)
add_custom_target(writeodf.h-target DEPENDS writeodf.h)
