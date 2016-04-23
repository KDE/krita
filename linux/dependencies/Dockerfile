# First stage. Builds everything but Krita itself. This exists because of the docker hub timeout
# which prevents us from building everyting in one go
FROM centos:6
ADD https://github.com/probonopd/AppImages/raw/master/recipes/krita/Recipe /Recipe
RUN export DO_NOT_BUILD_KRITA=1 && bash -ex Recipe && yum clean all && rm -rf /out && rm -rf /krita.appdir
