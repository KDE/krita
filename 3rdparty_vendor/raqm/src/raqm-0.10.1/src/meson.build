cc = meson.get_compiler('c')

config_h = configuration_data()

if cc.get_argument_syntax() == 'msvc'
    library_type = get_option('default_library')
    if library_type == 'shared' or library_type == 'both'
        config_h.set('RAQM_API', '__declspec (dllexport)')
    endif
endif

if sheenbidi.found()
    config_h.set('RAQM_SHEENBIDI', 1)
endif

configure_file(output: 'config.h', configuration: config_h)

version_h = configuration_data()
version_h.set('RAQM_VERSION_MAJOR', raqm_version[0].to_int())
version_h.set('RAQM_VERSION_MINOR', raqm_version[1].to_int())
version_h.set('RAQM_VERSION_MICRO', raqm_version[2].to_int())
version_h.set('RAQM_VERSION', meson.project_version())

configure_file(
    input: 'raqm-version.h.in',
    output: 'raqm-version.h',
    configuration: version_h,
    install: true,
    install_dir: join_paths(get_option('includedir')),
)

raqm_headers = files('raqm.h')
install_headers(raqm_headers)

libraqm = library(
    'raqm',
    'raqm.c',
    'raqm.h',
    version: meson.project_version(),
    dependencies: deps,
    c_args: ['-DHAVE_CONFIG_H'],
    install: true,
)

libraqm_test = static_library(
    'raqm-test',
    'raqm.c',
    'raqm.h',
    dependencies: deps,
    c_args: ['-DRAQM_TESTING', '-DHAVE_CONFIG_H'],
    install: false,
)

