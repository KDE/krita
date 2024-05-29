#!/bin/bash

usage=\
"Usage: $(basename "$0") [OPTION]...\n
Strips webp files: removes icc profile and removes animation\n
\n
where:\n
    -h,      --help              show this help text\n
    -d DIR,  --directory DIR     directory to search in
\n
"

DIR=$(pwd)

# Call getopt to validate the provided input.
options=$(getopt -o "hd:" --long "help directoyr:" -- "$@")
[ $? -eq 0 ] || {
    echo "Incorrect options provided"
    exit 1
}
eval set -- "$options"
while true; do
    case "$1" in
    -d | --directory)
        DIR=$2
        ;;
    -h | --help)
        echo -e $usage >&2
        exit 1
        ;;
    --)
        shift
        break
        ;;
    esac
    shift
done

for i in `find ${DIR} -name '*.webp'`; do 
    if webpmux -info $i | grep 'Number of' > /dev/null; then 
        echo "Animation found: $i"
        TMP_FILE=$(mktemp)
        webpmux -get frame 0 $i -o ${TMP_FILE}
        mv ${TMP_FILE} $i
    fi

    if webpinfo $i | grep ICCP > /dev/null; then
        echo "ICC found: $i"
        TMP_FILE=$(mktemp)
        webpmux -strip icc $i -o ${TMP_FILE}
        mv ${TMP_FILE} $i
    fi
done