#defines the set of products commonly wanted for classic Desktop environment with OS X
# TODO: platform specific things should be handled in toplevel CMakeLists.txt

calligra_define_productset(OSX "Calligra for OSX"
    OPTIONAL
        # apps
        FLOW
        KRITA
        #SHEETS
        STAGE
        WORDS
        KARBON
        # extras
        APP_CALLIGRA
        APP_CONVERTER
        FILEMANAGER
)
