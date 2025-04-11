# femboyOS
My attempt at an OS :3

## WARNING
This os is not ment to be used for real world usage. This is just a silly project i am making cuz i am bored. There are more bugs then i can count.

## How to build
1. Make the docker container using `docker build . --file buildenv/Dockerfile --tag femboyos:buildenv`.
2. Compile femboyOS using `docker run --rm -it -v .:/root/env femboyos:buildenv make build-x86_64`.
3. Done

## How to run
Either use some VM software or just boot it on bare metal.