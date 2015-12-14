#
# Helper function for extracting translatable messages from krita source code.
# Usage: krita_xgettext <pot-filename-without-path> <source-files-list>
# If there are no messages or the <source-files-list> is empty, the pot file is deleted.
#
# Example usage that creates $podir/myapp.pot file:
#     krita_xgettext myapp.pot `find . -name \*.cpp -o -name \*.h`
#
function krita_xgettext() {
    POTFILE="$podir/$1"
    shift
    if test -n "$*"; then
        krita_xgettext_internal $* | tee "${POTFILE}" | tail -n1 | grep "^msgstr \"\"\$" > /dev/null \
            || rm -f "${POTFILE}" 2> /dev/null
    fi
}

function krita_xgettext_internal() {
    SRC_FILES="$*"
    POT_PART_NORMAL="`mktemp $podir/_normal_XXXXXXXX.pot`"
    POT_PART_QUNDOFORMAT="`mktemp $podir/_qundoformat_XXXXXXXX.pot`"
    POT_PART_QUNDOFORMAT2="`mktemp $podir/_qundoformat2_XXXXXXXX.pot`"
    POT_MERGED="`mktemp $podir/_merged_XXXXXXXX.pot`"

    $XGETTEXT ${CXG_EXTRA_ARGS} ${SRC_FILES} -o "${POT_PART_NORMAL}" --force-po
    $XGETTEXT_PROGRAM --from-code=UTF-8 -C --kde -kkundo2_i18n:1 -kkundo2_i18np:1,2 -kkundo2_i18nc:1c,2 -kkundo2_i18ncp:1c,2,3 ${CXG_EXTRA_ARGS} ${SRC_FILES} -o "${POT_PART_QUNDOFORMAT}"

    if [ $(cat ${POT_PART_NORMAL} ${POT_PART_QUNDOFORMAT} | grep -c \(qtundo-format\)) != 0 ]; then
        echo "ERROR: Context '(qtundo-format)' should not be added manually. Use kundo2_i18n*() calls instead." 1>&2
        exit 17
    fi

    if [ -s "${POT_PART_QUNDOFORMAT}" ]; then
        # Prepend "(qtundo-format)" to existing msgctxt properties of messages
        sed -i -e 's/^msgctxt "/msgctxt "(qtundo-format) /' "${POT_PART_QUNDOFORMAT}"

        # Add msgctxt "(qtundo-format)" to messages not having msgctxt yet
        #
        # lastLine != "#, fuzzy" is the check for the .pot header.
        mv "${POT_PART_QUNDOFORMAT}" "${POT_PART_QUNDOFORMAT2}"
        cat "${POT_PART_QUNDOFORMAT2}" | awk '
            /^msgid "/ {
                if (lastLine !~ /^msgctxt/ && lastLine != "#, fuzzy") {
                    print "msgctxt \"(qtundo-format)\""
                }
            }
            { print ; lastLine = $0 }' > "${POT_PART_QUNDOFORMAT}"
    fi

    if [ -s "${POT_PART_NORMAL}" -a -s "${POT_PART_QUNDOFORMAT}" ]; then
        ${MSGCAT} -F "${POT_PART_NORMAL}" "${POT_PART_QUNDOFORMAT}" > ${POT_MERGED}
        MERGED_HEADER_LINE_COUNT=$(cat ${POT_MERGED} | grep "^$" -B 100000 --max-count=1 | wc -l)

        KDE_HEADER="$(cat ${POT_PART_NORMAL} | grep "^$" -B 100000 --max-count=1)"
        MERGED_TAIL="$(cat ${POT_MERGED} | tail -n +$MERGED_HEADER_LINE_COUNT)"

        # Print out the resulting .pot
        echo "$KDE_HEADER"
        echo "$MERGED_TAIL"
    elif [ -s "${POT_PART_NORMAL}" ]; then
        cat "${POT_PART_NORMAL}"
    elif [ -s "${POT_PART_QUNDOFORMAT}" ]; then
        cat "${POT_PART_QUNDOFORMAT}"
    fi

    rm -f "${POT_PART_NORMAL}" "${POT_PART_QUNDOFORMAT}" "${POT_PART_QUNDOFORMAT2}" "${POT_MERGED}"
}

# Sets EXCLUDE variable to excludes compatible with the find(1) command, e.g. '-path a -o -path b'.
# To unconditionally exclude dir (with subdirs) just put an empty file .i18n in it.
# To exclude dir for all translations but one, e.g. foo.pot, put a single "foo" line into the .i18n file.
function find_exclude() {
    EXCLUDE=""
    for f in `find . -name .i18n | sed 's/\/\.i18n$//g' | sort`; do
        if ! grep -q "^${1}$" "$f/.i18n" ; then
            if [ -n "$EXCLUDE" ] ; then EXCLUDE="$EXCLUDE -o " ; fi
            EXCLUDE="$EXCLUDE -path $f"
        fi
    done
    if [ -z "$EXCLUDE" ] ; then EXCLUDE="-path __dummy__" ; fi # needed because -prune in find needs args
}
