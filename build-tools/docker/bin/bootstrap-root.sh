#!/bin/bash

if [ -d ./persistent/krita ]; then
   for i in 'bin default-home'; do
       if [ ! -d "$i" ]; then
          mkdir $i
       fi
   done

   cp persistent/krita/build-tools/docker/bin/* ./bin/
   cp persistent/krita/build-tools/docker/default-home/{,.}* ./default-home/
   cp persistent/krita/build-tools/docker/Dockerfile ./
   cp persistent/krita/build-tools/docker/README.md ./
fi
