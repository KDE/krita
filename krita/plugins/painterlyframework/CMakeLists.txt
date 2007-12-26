add_subdirectory(tests)

#################################################
# Painterly Common Libraries

set(kritapainterlycommon_PART_SRCS
    kis_illuminant_profile.cpp
    kis_ks3_colorspace.cpp
    channel_converter.cpp
    cqp/cqpminimizer.c
    cqp/initial_point.c
    cqp/mg_pdip.c
    )

kde4_add_library(kritapainterlycommon SHARED ${kritapainterlycommon_PART_SRCS})
target_link_libraries(kritapainterlycommon kritaimage ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
install(TARGETS kritapainterlycommon DESTINATION ${LIB_INSTALL_DIR})
