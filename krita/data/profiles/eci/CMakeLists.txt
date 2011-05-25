file(GLOB ECI_profiles 
    "ECI_Offset_2009/*.icc"
    "ECI_Offset_cont_2004/*.icc"
    "eciRGBv10/*.icc"
    "eciRGB_v2_profile/*.icc"
)

install( FILES ${ECI_profiles} DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/color/icc/krita)



