FROM ubuntu:20.04

RUN apt update && apt -y install \
    binutils \
    build-essential \
    curl \
    gcc \
    git \
    grep \
    libc6-dev \
    make \
    man \
    procps \
    strace \
    vim \
    wget

WORKDIR /var/workspace/