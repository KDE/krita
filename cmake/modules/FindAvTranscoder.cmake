# AvTranscoder_FOUND
# AvTranscoder_INCLUDE_DIR
# AvTranscoder_LIBRARIES
# AvTranscoder_DEFINITIONS
# AvTranscoder_VERSION_MAJOR
# AvTranscoder_VERSION_MINOR
# AvTranscoder_VERSION_PATCH
# AvTranscoder_VERSION
# AvTranscoder_VERSION_STRING
# AvTranscoder_INSTALL_DIR
# AvTranscoder_LIB_DIR
# AvTranscoder_CMAKE_MODULES_DIR

find_package(AvTranscoder ${AvTranscoder_FIND_VERSION} QUIET NO_MODULE PATHS $ENV{HOME} /opt/AvTranscoder)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AvTranscoder CONFIG_MODE REQUIRED_VARS AvTranscoder_LIBRARIES)
