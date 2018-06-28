# Krita developer environment Docker image

This *Dockerfile* is based on the official KDE build environmet [0]
that in used on KDE CI for building official AppImage packages.
Therefore running this image in a docker container is the best way
to reproduce AppImage-only bugs in Krita.

[0] - https://binary-factory.kde.org/job/Krita_Nightly_Appimage_Dependency_Build/

## Prerequisites

Firstly make sure you have Docker installed

```bash
sudo apt install docker docker.io
```

Then you need to download deps and Krita source tree. These steps are not
included into the *Dockerfile* to save internal bandwidth (most Krita
developers already have al least one clone of Krita source tree).

```bash
# create directory structure for container control directory
mkdir -p krita-master/persistent
cd krita-master

# copy/chechout Krita sources to 'persistent/krita'
cp -r /path/to/sources/krita ./persistent/krita

# initialize control scripts and the Dockerfile
./persistent/krita/build-tools/docker/bin/bootstrap-root.sh

# download the deps archive
./bin/bootstrap-deps.sh
```

## Build the docker image and run the container

```bash
./bin/build_image krita-deps
./bin/run_container krita-deps krita-master
```

## Enter the container and build Krita

```bash
# enter the docker container (the name will be
# fetched automatically from '.container_name' file)

./bin/enter

# ... now your are inside the container with all the deps prepared ...

# build Krita as usual
cd appimage-workspace/krita-build/
run_cmake.sh ~/persistent/krita
make -j8 install

# start Krita
krita

```

## Extra developer tools

To install QtCreator, enter container and start the installer, downloaded while
fetching dependencies. Make sure you install it into '~/qtcreator' directory
without any version suffixes, then you will be able to use the script below:

```bash
# inside the container
./persistent/qt-creator-opensource-linux-x86_64-4.6.2.run
```

To start QtCreator:

```bash
# from the host
./bin/qtcreator
```

## Stopping the container and cleaning up

When not in use you can stop the container. All your filesystem state is saved, but
all the currently running processes are killed (just ensure you logout from all the
terminals before stopping).

```bash
# stop the container
./bin/stop

# start the container
./bin/start
```

If you don't need your container/image anymore, you can delete them from the docker

```bash
# remove the container
sudo docker rm krita-master

# remove the image
sudo docker rmi krita-deps
```

TODO: do we need some extra cleaups for docker's caches?


## Troubleshooting

### Krita binary is not found after the first build

Either relogin to the container or just execute `source ~/.devenv.inc`


### Not enough space on root partition

All the docker images and containers are stored in a special docker-daemon controlled
folder under */var* directory. You might not have enough space there for building Krita
(it needs about 10 GiB). In such a case it is recommended to move the docker images
folder into another location, where there is enough space.

```bash
echo 'DOCKER_OPTS="-g /home/devel5/docker"' >> /etc/default/docker
```
