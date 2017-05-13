

set(userfeedback_core_srcs
    abstractdatasource.cpp
    applicationversionsource.cpp
    compilerinfosource.cpp
    cpuinfosource.cpp
    localeinfosource.cpp
    openglinfosource.cpp
    platforminfosource.cpp
    propertyratiosource.cpp
    provider.cpp
    qtversionsource.cpp
    screeninfosource.cpp
    selectionratiosource.cpp
    startcountsource.cpp
    surveyinfo.cpp
    usagetimesource.cpp
)
if(Qt5Core_FOUND)
    list(APPEND userfeedback_core_srcs ${userfeedback_core_QM_LOADER})
endif()

add_library(UserFeedbackCore SHARED ${userfeedback_core_srcs})
set_target_properties(UserFeedbackCore PROPERTIES
    SOVERSION ${USERFEEDBACK_VERSION}
    VERSION ${USERFEEDBACK_VERSION}
)
generate_export_header(UserFeedbackCore)

if(Qt5Core_FOUND)
    target_link_libraries(UserFeedbackCore PUBLIC Qt5::Core PRIVATE Qt5::Gui Qt5::Network)
    target_compile_definitions(UserFeedbackCore PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0x040800)
else()
    target_link_libraries(UserFeedbackCore PUBLIC ${QT_QTCORE_LIBRARIES} PRIVATE ${QT_QTGUI_LIBRARIES} ${QT_QTNETWORK_LIBRARIES})
endif()
target_link_libraries(UserFeedbackCore PRIVATE UserFeedbackCommon)
target_include_directories(UserFeedbackCore PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};${CMAKE_SOURCE_DIR}/src>")
target_include_directories(UserFeedbackCore INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/UserFeedback;${KDE_INSTALL_INCLUDEDIR}>")

install(TARGETS UserFeedbackCore EXPORT UserFeedbackTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${UserFeedbackCore_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/userfeedbackcore_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/UserFeedback
)



