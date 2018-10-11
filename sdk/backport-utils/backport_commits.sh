#!/bin/bash

# A POSIX variable
# Reset in case getopts has been used previously in the shell.
OPTIND=1

# Initialize our own variables:
ALL_COMMITS=0
SINCE_ARG=
UNTIL_ARG=
ARS=()

OPTIONS=as:u:

function print_usage {
    echo "Usage: $0 [-a] [-s SINCE] [-u UNTIL]"
}

while getopts "h?as:u:" opt; do
    case "$opt" in
        h|\?)
            print_usage
            exit 0
            ;;
        a)
            ALL_COMMITS=1
            ;;
        s)
            ARGS+=(--since="$OPTARG")
            ;;
        u)
            ARGS+=(--until="$OPTARG")
            ;;
        *)
            print_usage
            exit 1
            ;;
    esac
done

shift $((OPTIND-1))
[ "${1:-}" = "--" ] && shift

git fetch

BRANCHNAME=`git rev-parse --abbrev-ref HEAD`
INITIAL_SHA1=`git rev-parse HEAD`

LOGFILE=`tempfile -p log`
COMMITSLIST=`tempfile -p commits`
PICKCOMMITSLIST=`tempfile -p pick`

if [ $ALL_COMMITS -gt 0 ]; then
    # check for **all** commits that were not yet backported from master
    git log --right-only --no-merges --pretty=medium --cherry-pick \
        "${ARGS[@]}" \
        HEAD...origin/master > $LOGFILE
else
    # check for commits containing BACKPORT keyword that were not
    # yet backported from master
    git log --right-only --no-merges --pretty=medium --cherry-pick \
        "${ARGS[@]}" \
        --grep="BACKPORT:.*\\b$BRANCHMANE\\b.*" HEAD...origin/master > $LOGFILE
fi

sed  -E '/^commit [0-9a-f]+$/!s/^/#/' $LOGFILE > $COMMITSLIST

$EDITOR $COMMITSLIST

sed  -nE 's/^commit ([0-9a-f]+)$/\1/p' $COMMITSLIST > $PICKCOMMITSLIST

if [ ! -s $PICKCOMMITSLIST ]; then
    echo "List of cherry-picked commits is empty. Aborting..."
else
    NUMCOMMITS=$(wc -l $PICKCOMMITSLIST | cut -d' ' -f1)
    echo "Start cherry picking $NUMCOMMITS commits"
    cat $PICKCOMMITSLIST | xargs git cherry-pick -x
fi

rm $LOGFILE
rm $COMMITSLIST
rm $PICKCOMMITSLIST

