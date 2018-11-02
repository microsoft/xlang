FROM ubuntu:18.04

RUN apt-get update

RUN apt-get install apt-utils -y

RUN apt-get install -y --no-install-recommends \
    apt-transport-https \
    ca-certificates \
    curl \
    file \
    ftp \
    git \
    gnupg \
    iproute2 \
    iputils-ping \
    locales \
    lsb-release \
    nodejs \
    sudo \
    time \
    unzip \
    wget \
    zip

RUN wget -q https://packages.microsoft.com/config/ubuntu/18.04/packages-microsoft-prod.deb \
    && dpkg -i packages-microsoft-prod.deb \
    && apt-get update \
    && apt-get -y install dotnet-sdk-2.1

RUN apt-get install -y \
    clang \
    cmake \
    ninja-build \
    libc++-dev \
    libc++abi-dev

ENV DOTNET_RUNNING_IN_CONTAINER=true \
    DOTNET_USE_POLLING_FILE_WATCHER=true \
    NUGET_XMLDOC_MODE=skip