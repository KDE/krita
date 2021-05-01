#!/usr/bin/env zsh
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

# Simple script to test all .app test's
#
# 1. Find any ./test/*.app
# 2. copies file to install folder (Assumed i/bin)
# 3. Excutes test
# 4. report back pass/fail status
#

if test -z $BUILDROOT; then
    echo "ERROR: BUILDROOT env not set, exiting!"
    echo "\t Must point to the root of the buildfiles as stated in 3rdparty Readme"
    exit
fi

BUILDROOT="${BUILDROOT%/}"
INSTALL_DIR="${BUILDROOT}/i/bin"
WORK_DIR=${PWD}

# If current directory inside buildroot
if [[ ! $(grep "${BUILDROOT}" <<< "${PWD}") ]] ; then
    WORK_DIR_CHECK="TRUE"
fi

SKIP_TESTS=(\
    "KisRandomGeneratorDemo.app"\
    "storedroptest.app"\
    "kis_action_manager_test.app"\
    "kis_node_manager_test.app"\
    "kis_selection_manager_test.app"\
    "KisActionManagerTest.app"\
    "KisNodeManagerTest.app"\
    "KisSelectionManagerTest.app"\
    "KisZoomAndPanTest.app"\
    "kis_crash_filter_test.app"\
    "FreehandStrokeBenchmark.app")


find_execute_tests() {

    local test_found=($(find "${WORK_DIR}" -type d -name "*.app"))
    local total_tests=${#test_found[@]}

    if [[ ${#} > 0 ]]; then 
        local specifyc_tests="TRUE"
        local total_tests=${#}
    fi


    local failed_tests=()
    local fails=0
    local i=1

    for FILE_PATH in "${test_found[@]}"; do
        if [[ $(grep "tests" <<< "${FILE_PATH}") ]]; then
            
            local app_name="$(basename ${FILE_PATH})"

            # if names supplied skip any test not in args
            if [[ -n "${specifyc_tests}" && ! "${@[@]}" =~ "${app_name%%.app}" ]]; then
                continue
            fi

            # skip GUI locking tests
            # if ((${SKIP_TESTS[(Ie)${foo}]})) ; then ## zsh
            if [[ "${SKIP_TESTS[@]}" =~ "${app_name}" ]]; then
                echo "skipping ------------------ ${app_name}"
                continue
            fi

            rsync -aq "${FILE_PATH}/" "${INSTALL_DIR}/${app_name}"
            printf "\tStart %d: %s\n" ${i} "${FILE_PATH}"
            printf "%d/%d\t%s ...  " ${i} ${total_tests} "${app_name}"

            "${INSTALL_DIR}/${app_name}/Contents/MacOS/${app_name%%.app}" &> /dev/null

            if [[ ${?} != 0 ]]; then
                printf "\e[31m%s\e[0m\n" "Fail!"
                failed_tests+=("${app_name}")
                fails=$((${fails} + 1))
            else
                echo "Passed"
            fi
            local i=$((${i} + 1))
        else
            echo "skipping ------------------ ${FILE_PATH}"
        fi
    done

    # failed_tests_array=("${failed_tests}")

    printf "\n%d tests out of %s failed\n\n" ${#failed_tests[@]} ${total_tests}
    echo "Failed tests:"
    printf '\t\e[31m%s\e[0m\n' "${failed_tests[@]}"

}

echo "BUILDROOT set to ${BUILDROOT}"

# -- Parse input args
for arg in "${@}"; do
    if [[ ${arg} = --install_dir=* ]]; then
        INSTALL_DIR="${arg#*=}"
    elif [[ ${arg} = --work_dir=* ]]; then
        WORK_DIR="${arg#*=}"
        WORK_DIR_CHECK="SET"
    else
        parsed_args="${parsed_args} ${arg}"
    fi
done


if [[ -n "${WORK_DIR_CHECK}" && "${WORK_DIR_CHECK}" != "SET" ]] ; then
    echo "Current directory not inside buildroot"
    echo "use --work_dir=<path> to set custom"
    exit
fi

find_execute_tests ${parsed_args}
