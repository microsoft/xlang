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

RUN apt-get install -y \
    clang \
    cmake \
    ninja-build \
    libc++-dev \
    libc++abi-dev