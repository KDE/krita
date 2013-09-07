# disable QStringBuilder for kdchart, not yet optimized for
remove_definitions(
    -DQT_USE_FAST_CONCATENATION
    -DQT_USE_FAST_OPERATOR_PLUS
)

add_subdirectory(src)


