#!/bin/bash

set -ex

IMAGE=ubuntu:18.04
CONTAINER=container-krita-snap

function at_exit {
  lxc stop $CONTAINER
}

lxc stop $CONTAINER || true

lxc launch --ephemeral "$IMAGE" $CONTAINER
sleep 4 # so network is up

trap at_exit INT TERM EXIT

lxc file push -p --recursive . $CONTAINER/workspace
lxc exec $CONTAINER -- /workspace/snap/build_in_container.sh
lxc file pull --recursive $CONTAINER/workspace/snap/result .
