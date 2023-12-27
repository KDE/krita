#!/usr/bin/env zsh
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

# creates a universal tar build from build-x86_64 and build-arm64
# First argument is <root_dir> where build dirs reside.

consolidate_universal_binaries () {
    # echo "dirs ${DIR_X86} :: ${DIR_UNI}"
    for f in "${@}"; do
        # Only try to consolidate Mach-O and static libs
        if [[ -n $(file "${f}" | grep "Mach-O") || "${f:(-2)}" = ".a" ]]; then
            # echo "${BUILDROOT}/${DEPBUILD_X86_64_DIR}/${f##*test-i/}"
            LIPO_OUTPUT=$(lipo -info "${f}" | grep Non-fat 2> /dev/null)
            if [[ -n ${LIPO_OUTPUT} ]]; then
                if [[ -f "${DIR_X86}/${f##*${DIR_UNI}/}" ]]; then
                    echo "creating universal binary -- ${f##*${DIR_UNI}/}"
                    lipo -create "${f}" "${DIR_X86}/${f##*${DIR_UNI}/}" -output "${f}"
                fi
            fi
            # log "ignoring ${f}"
        fi
    done
}

consolidate_build_dirs() {
    DIR_X86=${1}
    local DIR_ARM=${2}
    local DIR_UNI=${3}

    rsync -aq ${DIR_ARM}/ ${DIR_UNI}
    consolidate_universal_binaries $(find "${DIR_UNI}" -type f)
}

if [[ ${#@} < 1 ]]; then
    echo "script needs root_dir of builds to consolidate as input"
    exit
fi

root_dir=${1}

if [[ ! -d ${root_dir} ]]; then
    echo "provided path is not a directory"
    echo "path: ${root_dir}"
fi

dir_x86="${root_dir}/build-x86_64"
dir_arm="${root_dir}/build-arm64"
dir_uni="${root_dir}/build_uni"


if [[ -d "${dir_x86}" && -d "${dir_arm}" ]]; then
    consolidate_build_dirs "${dir_x86}" "${dir_arm}" "${dir_uni}"
else
    if [[ -d "${dir_x86}" ]]; then
        rsync -aq "${dir_x86}/" "${dir_uni}"
    else
        rsync -aq "${dir_arm}/" "${dir_uni}"
    fi
fi

# from https://www.gnu.org/software/automake/manual/html_node/DESTDIR.html
build_tar=package_build_fat.tar.gz

cd "${dir_uni}"
find . '(' -type f -o -type l ')' -print > ../files.lst
tar zcvf "${build_tar}" --files-from "../files.lst"
mv ${build_tar} ../

echo "consolidate end"