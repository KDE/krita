#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${DIR}/find_default_container_file.inc

container_name=$(parseContainerArgs $*)
if [ -z ${container_name} ]; then
    exit 1
fi

if [ ! -e /proc/driver/nvidia/version ]; then
    echo "Cannot find NVIDIA bestion file: /proc/driver/nvidia/version"
    exit 1
fi

nvidia_version=$(cat /proc/driver/nvidia/version | head -n 1 | awk '{ print $8 }')
driver_file=NVIDIA-Linux-x86_64-${nvidia_version}.run
driver_url=http://download.nvidia.com/XFree86/Linux-x86_64/${nvidia_version}/${driver_file}

if [ ! -f persistent/${driver_file} ]; then
    (
        cd persistent
        wget http://download.nvidia.com/XFree86/Linux-x86_64/${nvidia_version}/${driver_file} || exit 1
        chmod a+x ${driver_file}
    )
fi

if [ -f persistent/${driver_file} ]; then
    sudo docker exec -ti -u root ${container_name} /home/appimage/persistent/${driver_file} -a -N --ui=none --no-kernel-module -s
else
    echo "Cannot find the driver file: ${driver_file}"
    exit 1
fi
