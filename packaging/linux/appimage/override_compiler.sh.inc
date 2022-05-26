#!/bin/bash

if [[ -x /usr/bin/gcc-11 && -x /usr/bin/g++-11 ]] ; then
    export CC=/usr/bin/gcc-11
    export CXX=/usr/bin/g++-11
fi
