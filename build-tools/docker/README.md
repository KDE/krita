# Krita developer environment Docker image

This *Dockerfile* is based on the official KDE build environmet [0]
that in used on KDE CI for building official AppImage packages.
Therefore running this image in a docker container is the best way
to reproduce AppImage-only bugs in Krita.

[0] - https://binary-factory.kde.org/job/Krita_Nightly_Appimage_Dependency_Build/

## Prerequisites

Firstly we need to download deps and Krita source tree. These steps are not
included into the *Dockerfile* to save internal bandwidth (most Krita
developers already have al least one clone of Krita source tree).

```bash
# download the deps archive
./bin/bootstrap-deps.sh

# mount/copy/chechout Krita sources to 'persistent/krita'
mkdir -p persistent/krita
sudo mount --bind ../../ ./persistent/krita
```

## Build the docker image and run the container

```bash
./bin/build_image krita-deps
./bin/run_container krita-deps krita-build
```

## Enter the container and build Krita

```bash
# enter the docker container
./bin/enter_container krita-build

# ... now your are inside the container with all the deps prepared ...

# build Krita as usual
cd appimage-workspace/krita-build/
run_cmake.sh ~/persistent/krita
make -j8 install

# start Krita
krita

```

## Extra developer tools

If you want to develop Krita, you might want to install at least some
developer tools into the container, e.g. GDB, Valgring, ccmake and QtCreator.
To do that, execute the following from yout **host** console:

```bash
sudo docker exec -ti krita-build apt install gdb
sudo docker exec -ti krita-build apt install valgrind
sudo docker exec -ti krita-build apt install cmake-curses-gui

# inside the container
cd ~/persistent
wget http://master.qt.io/archive/online_installers/3.0/qt-unified-linux-x64-3.0.4-online.run
./qt-unified-linux-x64-3.0.4-online.run

# when going through the setup wizard select not to install any
# extra Qt libraries, install QtCreator only!

```

## Stopping the container and cleaning up

When not in use you can stop the container. All your filesystem state is saved, but
all the currently running processes are killed (just ensure you logout from all the
terminals before stopping).

```bash
sudo docker stop krita-build
```

If you don't need your container/image anymore, you can delete them from the docker

```bash
# remove the container
sudo docker rm krita-build

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
