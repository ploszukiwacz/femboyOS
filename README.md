# femboyOS
My attempt at an OS :3

## How to build
1. Make the docker container using `docker build . --file buildenv/Dockerfile --tag femboyos:buildenv`.
2. Compile femboyOS using `docker run --rm -it -v .:/root/env femboyos:buildenv make build-x86_64`.
3. Done