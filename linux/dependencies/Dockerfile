# First stage. Builds everything but Krita itself. This exists because of the docker hub timeout
# which prevents us from building everyting in one go
FROM centos:6
ADD https://raw.githubusercontent.com/KDE/krita/master/linux/Recipe /Recipe
RUN export DO_NOT_BUILD_KRITA=1 && bash -ex Recipe && yum clean all && rm -rf /out && rm -rf /krita.appdir
