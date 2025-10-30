kis_add_library(dummytrivialinterface SHARED DummyTrivialInterface.cpp)
target_link_libraries(dummytrivialinterface PRIVATE Qt::Core)
generate_export_header(dummytrivialinterface BASE_NAME dummytrivialinterface)

macro(add_dummy_plugin target_name plugin_id plugin_version plugin_json_file)
    set(dummy_plugin_SRCS
        DummyPlugin.cpp
    )

    kis_add_library(${target_name} MODULE ${dummy_plugin_SRCS})

    set_target_properties(${target_name}
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dummyplugins
        )

    target_compile_definitions(${target_name}
        PRIVATE
        DUMMY_PLUGIN_ID="${plugin_id}"
        DUMMY_PLUGIN_VERSION=${plugin_version}
        DUMMY_PLUGIN_JSON_FILE="${plugin_json_file}"
        )

    target_link_libraries(${target_name}
        PRIVATE
            Qt::Core
            kritaglobal
            KF${KF_MAJOR}::CoreAddons
            dummytrivialinterface
    )
endmacro()

add_dummy_plugin(kritadummyplugin1 "dummyplugin1" 1 "dummyplugin1.json")
add_dummy_plugin(kritadummyplugin2 "dummyplugin2" 1 "dummyplugin2.json")
add_dummy_plugin(kritadummyplugin2_ver2 "dummyplugin2" 2 "dummyplugin2_ver2.json")
add_dummy_plugin(kritadummyplugin3 "dummyplugin3" 3 "dummyplugin3_ver_as_int.json")

kis_add_test(
    KoPluginLoaderTest.cpp
    TEST_NAME KoPluginLoaderTest
    LINK_LIBRARIES kritaplugin kritaglobal kritatestsdk KF${KF_MAJOR}::CoreAddons KF${KF_MAJOR}::ConfigCore dummytrivialinterface
)

target_compile_definitions(KoPluginLoaderTest
    PRIVATE
    DUMMY_PLUGINS_PATH=${CMAKE_CURRENT_BINARY_DIR}/dummyplugins
    )

add_dependencies(KoPluginLoaderTest kritadummyplugin1 kritadummyplugin2 kritadummyplugin2_ver2 kritadummyplugin3)