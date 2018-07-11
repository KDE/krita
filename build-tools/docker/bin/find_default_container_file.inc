#!/bin/bash

function findContainerName {
    DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    container_file=`${DIR}/find_up ./ -name .container_name`

    if [ ! -z ${container_file} ]; then
        cat ${container_file}
    fi
}

function parseContainerArgs {
    container_name=

    if [ "$#" -ne 1 ]; then
        if [ "$#" -eq 0 ]; then
            DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
            container_name=$(findContainerName)
        fi

        if [ -z ${container_name} ]; then
            echo "Usage: $0 CONTAINER_NAME" >&2
            exit 1
        fi
    else
        container_name=$1
    fi
    echo ${container_name}
}

