#defines the set of products commonly wanted for classic Desktop environment with OS X
# TODO: platform specific things should be handled in toplevel CMakeLists.txt

calligra_define_productset(DESKTOP "Calligra for Desktop"
    OPTIONAL
        # apps
        AUTHOR
        FLOW
        #    KEXI
        KRITA
        SHEETS
        STAGE
        WORDS
        BRAINDUMP
        KARBON
        # extras
        APP_CALLIGRA
        APP_CONVERTER
        FILEMANAGER
)
