FROM kdeorg/appimage-base

MAINTAINER Dmitry Kazakov <dimula73@gmail.com>
RUN apt-get update && apt-get -y install curl

ENV USRHOME=/home/appimage

RUN chsh -s /bin/bash appimage

RUN locale-gen en_US.UTF-8

USER appimage

RUN echo 'export LC_ALL=en_US.UTF-8' >> ${USRHOME}/.bashrc && \
    echo 'export LANG=en_US.UTF-8'  >> ${USRHOME}/.bashrc && \
    echo "export PS1='\u@\h:\w>'"  >> ${USRHOME}/.bashrc && \
    echo 'source ~/devenv.inc' >> ${USRHOME}/.bashrc && \
    echo 'prepend PATH ~/bin/' >> ${USRHOME}/.bashrc

RUN mkdir -p ${USRHOME}/appimage-workspace/krita-inst && \
    mkdir -p ${USRHOME}/appimage-workspace/krita-build && \
    mkdir -p ${USRHOME}/bin

COPY ./default-home/devenv.inc \
     ./default-home/.bash_aliases \
     ${USRHOME}/

COPY ./default-home/run_cmake.sh \
     ${USRHOME}/bin

ADD persistent/krita-appimage-deps.tar ${USRHOME}/appimage-workspace/

CMD tail -f /dev/null


