# sudo rm -rf build/ dist/
docker run --rm -it -v .:/root/env femboyos:buildenv make build-x86_64
