# Due to the timeout of docker hub, we cannot build the docker image in one go.
# Hence we base this recipe on another docker image, krita-dependencies
FROM probonopd/appimages:krita-dependencies
ADD https://raw.githubusercontent.com/KDE/krita/master/linux/Recipe /Recipe
RUN bash -ex Recipe && yum clean all && rm -rf /out && rm -rf /krita.appdir /Krita*AppImage
