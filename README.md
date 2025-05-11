# This project is archived, see [femboyOS-v2](https://github.com/ploszukiwacz/femboyOS-v2) for the new version!

# femboyOS
My attempt at an OS :3

## WARNING
This os is not ment to be used for real world usage. This is just a silly project i am making cuz i am bored. There are more bugs then i can count.

## How to build
1. Make the docker container using `docker pull ghcr.io/ploszukiwacz/femboyos-buildenv:latest`.
2. Compile femboyOS using `docker run --rm -it -v .:/root/env ghcr.io/ploszukiwacz/femboyos-buildenv:latest make build-x86_64`.
3. Done

## How to run
Either use some VM software or just boot it on bare metal.
