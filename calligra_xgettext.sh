#
# Helper function for extracting translatable messages from Calligra source code
# Usage example:
#     calligra_xgettext planlibs.pot "`find . -name \*.cpp -o -name \*.h`"
#
function calligra_xgettext() {
    SRC_FILES="$*"
    POT_PART_NORMAL="`mktemp $podir/_normal_XXXXXXXX.pot`"
    POT_PART_QUNDOFORMAT="`mktemp $podir/_qundoformat_XXXXXXXX.pot`"
    POT_MERGED="`mktemp $podir/_merged_XXXXXXXX.pot`"

    $XGETTEXT ${CXG_EXTRA_ARGS} ${SRC_FILES} -o "${POT_PART_NORMAL}"
    $XGETTEXT_PROGRAM --from-code=UTF-8 -C --kde -kkundo2_i18n:1 -kkundo2_i18np:1,2 -kkundo2_i18nc:1c,2 -kkundo2_i18ncp:1c,2,3 ${CXG_EXTRA_ARGS} ${SRC_FILES} -o "${POT_PART_QUNDOFORMAT}"

    if [ $(cat ${POT_PART_NORMAL} ${POT_PART_QUNDOFORMAT} | grep -c \(qtundo-format\)) != 0 ]; then
        echo "ERROR: Context '(qtundo-format)' should not be added manually. Use kundo2_i18n*() calls instead." 1>&2
        exit 17
    fi

    if [ -f "${POT_PART_QUNDOFORMAT}" ]; then
        # Prepend "(qtundo-format)" to existing msgctxt properties of messages
        sed -i -e 's/^msgctxt "/msgctxt "(qtundo-format) /' "${POT_PART_QUNDOFORMAT}"

        # Add msgctxt "(qundo-format)" to messages not having msgctxt yet
        sed -i -e '/^#/{n;/msgid "/i \msgctxt "(qtundo-format)"
        }' "${POT_PART_QUNDOFORMAT}"
    fi

    if [[ -f "${POT_PART_NORMAL}" && -f "${POT_PART_QUNDOFORMAT}" ]]; then
        ${MSGCAT} -F "${POT_PART_NORMAL}" "${POT_PART_QUNDOFORMAT}" > ${POT_MERGED}
        MERGED_HEADER_LINE_COUNT=$(cat ${POT_MERGED} | grep "^$" -B 100000 --max-count=1 | wc -l)

        KDE_HEADER="$(cat ${POT_PART_NORMAL} | grep "^$" -B 100000 --max-count=1)"
        MERGED_TAIL="$(cat ${POT_MERGED} | tail -n +$MERGED_HEADER_LINE_COUNT)"

        # Print out the resulting .pot
        echo "$KDE_HEADER"
        echo "$MERGED_TAIL"
    elif [ -f "${POT_PART_NORMAL}" ]; then
        cat "${POT_PART_NORMAL}"
    elif [ -f "${POT_PART_QUNDOFORMAT}" ]; then
        cat "${POT_PART_QUNDOFORMAT}"
    fi

    rm -f "${POT_PART_NORMAL}" "${POT_PART_QUNDOFORMAT}" "${POT_MERGED}"
}
