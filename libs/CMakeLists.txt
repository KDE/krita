OPTION(KOFFICE_ENABLE_SCRIPTING   "build scripting library (Kross)" ON)
#OPTION(KOFFICE_ENABLE_SCRIPTING   "build scripting library (Kross)" OFF)

add_subdirectory( store )
add_subdirectory( kwmf )
add_subdirectory( kofficecore )
add_subdirectory( kofficeui )
add_subdirectory( koproperty )
add_subdirectory( kotext )
add_subdirectory( kformula )
add_subdirectory( kopainter )
add_subdirectory( flake )
add_subdirectory( pigment )
add_subdirectory( kopageapp )

if(KOFFICE_ENABLE_SCRIPTING)
    # the kokross directory contains extensions for koffice scripting with Kross
    add_subdirectory(kokross)
    # following directory will be moved to kdebindings
    add_subdirectory(kross)
endif(KOFFICE_ENABLE_SCRIPTING)

