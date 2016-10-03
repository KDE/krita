To compile Krita from Git as an appimage:


- Create a fresh CentOS 6.x environment:
  - VM:
    Install from CentOS-6.8-x86_64-bin-DVD1.iso
    yum install git
    git clone git://anongit.kde.org/krita.git /krita/
  - Docker:
    docker pull centos:6
    cd $HOME/krita/  # assuming "Building Krita 3.0 on Linux for cats" layout
    docker run -ti -v $(pwd)/src:/krita centos:6 /bin/bash


- Set Git to the right branch, e.g.:
  cd /krita/
  git checkout krita/3.0.1
  # Caution: uncommitted changes will be removed during the build


- Install the dependencies
  cd /krita/packaging/linux/appimage/
  bash build-deps.sh  # ~2-3h, 5-6GB
  
  Notes:
  - You can skip this next time your rebuild
  - If using a VM, you could make a snapshot at this point
  - If using docker, you could `docker commit XXXXXX && docker tag YYYYYY krita-deps`
    Then at each build:
    docker run -ti -v $(pwd)/src:/krita -v $(pwd)/build:/krita_build -v $(pwd)/out:/out krita-deps /bin/bash


- Build Krita:
  cd /krita/packaging/linux/appimage/
  bash build-krita.sh  # ~1h


- Bundle the appimage:
  bash build-image.sh  # ~15mn


Your appimage is ready in /out/krita-3.X.X-yyyyyyy-x86_64.appimage !
