# Due to the timeout of docker hub, we cannot build the docker image in one go.
# Hence we base this recipe on another docker image, krita-dependencies
FROM probonopd/appimages:krita-dependencies
ADD https://github.com/probonopd/AppImages/raw/master/recipes/krita/Recipe /Recipe
RUN bash -ex Recipe && yum clean all && rm -rf /out && rm -rf /krita.appdir /Krita*AppImage
