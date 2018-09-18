#
# Helper function for extracting translatable messages from Calligra/Krita/Kexi source code.
# Usage: kundo2_aware_xgettext <pot-filename-without-path> <source-files-list>
# If there are no messages or the <source-files-list> is empty, the pot file is deleted.
#
# Example usage that creates $podir/myapp.pot file:
#     kundo2_aware_xgettext myapp.pot `find . -name \*.cpp -o -name \*.h`
#
function kundo2_aware_xgettext() {
    POTFILE="$podir/$1"
    shift
    if test -n "$*"; then
        # we rely on last line being a 'msgstr' signaling that strings has been extracted (a header is always present)
        # normally it ends with 'msgstr ""' but if plural it can end with eg 'msgstr[1] ""'
        kundo2_aware_xgettext_internal $* | tee "${POTFILE}" | tail -n1 | grep "^msgstr" > /dev/null \
            || rm -f "${POTFILE}" 2> /dev/null
    fi
}

# How to unit test:
#   export podir=.
#   cp init-sample.pot sample.pot
#   source krita_xgettext.sh
#   add_ctxt_qtundo sample.pot
#
#   Then check that all messages in sample.pot have "(qtundo-format)" in msgctxt.
function add_ctxt_qtundo() {
    POT_PART_QUNDOFORMAT="$1"
    POT_PART_QUNDOFORMAT2="`mktemp $podir/_qundoformat2_XXXXXXXX.pot`"

    # Prepend "(qtundo-format)" to existing msgctxt properties of messages
    sed -i -e 's/^msgctxt "/msgctxt "(qtundo-format) /' "${POT_PART_QUNDOFORMAT}"

    # Add msgctxt "(qtundo-format)" to messages not having msgctxt yet
    #
    # lastLine != "#, fuzzy" is the check for the .pot header.
    # If lastLine starts with '"' the msgctxt has been split on several lines and is treated by sed above, so skip it
    mv "${POT_PART_QUNDOFORMAT}" "${POT_PART_QUNDOFORMAT2}"
    < file "${POT_PART_QUNDOFORMAT2}" | awk '
        /^msgid "/ {
            if (lastLine !~ /^\"/ && lastLine !~ /^msgctxt/ && lastLine != "#, fuzzy") {
                print "msgctxt \"(qtundo-format)\""
            }
        }
        { print ; lastLine = $0 }' > "${POT_PART_QUNDOFORMAT}"

    rm -f "${POT_PART_QUNDOFORMAT2}"
}

function kundo2_aware_xgettext_internal() {
    SRC_FILES="$*"
    POT_PART_NORMAL="`mktemp $podir/_normal_XXXXXXXX.pot`"
    POT_PART_QUNDOFORMAT="`mktemp $podir/_qundoformat_XXXXXXXX.pot`"
    POT_MERGED="`mktemp $podir/_merged_XXXXXXXX.pot`"

    $XGETTEXT ${CXG_EXTRA_ARGS} ${SRC_FILES} -o "${POT_PART_NORMAL}" --force-po

    XGETTEXT_FLAGS_KUNDO2="\
--copyright-holder=This_file_is_part_of_KDE \
--msgid-bugs-address=http://bugs.kde.org \
--from-code=UTF-8
-C -k --kde \
-kkundo2_i18n:1 -kkundo2_i18np:1,2 -kkundo2_i18nc:1c,2 -kkundo2_i18ncp:1c,2,3 \
"

    $XGETTEXT_PROGRAM ${XGETTEXT_FLAGS_KUNDO2} ${CXG_EXTRA_ARGS} ${SRC_FILES} -o "${POT_PART_QUNDOFORMAT}"

    if [ $(cat ${POT_PART_NORMAL} ${POT_PART_QUNDOFORMAT} | grep -c \(qtundo-format\)) != 0 ]; then
        echo "ERROR: Context '(qtundo-format)' should not be added manually. Use kundo2_i18n*() calls instead." 1>&2
        exit 17
    fi

    if [ -s "${POT_PART_QUNDOFORMAT}" ]; then
        add_ctxt_qtundo "${POT_PART_QUNDOFORMAT}"
    fi

    if [ -s "${POT_PART_NORMAL}" -a -s "${POT_PART_QUNDOFORMAT}" ]; then
        # ensure an empty line or else KDE_HEADER search will fail
        # in case POT_PART_NORMAL only contains header
        echo "" >>${POT_PART_NORMAL}
        
        ${MSGCAT} -F "${POT_PART_NORMAL}" "${POT_PART_QUNDOFORMAT}" > ${POT_MERGED}
        MERGED_HEADER_LINE_COUNT=$(< file ${POT_MERGED} grep "^$" -B 100000 --max-count=1 | wc -l)
        KDE_HEADER="$(< file ${POT_PART_NORMAL} grep "^$" -B 100000 --max-count=1)"
        MERGED_TAIL="$(< file ${POT_MERGED} tail -n +$MERGED_HEADER_LINE_COUNT)"

        # Print out the resulting .pot
        echo "$KDE_HEADER"
        echo "$MERGED_TAIL"
    elif [ -s "${POT_PART_NORMAL}" ]; then
        echo "# POT_PART_NORMAL only"
        cat "${POT_PART_NORMAL}"
    elif [ -s "${POT_PART_QUNDOFORMAT}" ]; then
        echo "# POT_PART_QUNDOFORMAT only"
        cat "${POT_PART_QUNDOFORMAT}"
    fi

    rm -f "${POT_PART_NORMAL}" "${POT_PART_QUNDOFORMAT}" "${POT_MERGED}"
}

# Sets EXCLUDE variable to excludes compatible with the find(1) command, e.g. '-path a -o -path b'.
# To unconditionally exclude dir (with subdirs) just put an empty file .i18n in it.
# To disable excluding for given file, e.g. foo.pot, add "foo.pot" line to the .i18n file.
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
