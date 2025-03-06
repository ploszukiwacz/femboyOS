import { run } from 'pierre';

async function makeDockerContainer() {
  await run('docker build . --file buildenv/Dockerfile --tag femboyos:buildenv');
}

async function compile() {
  await run('docker run --rm -v $(pwd):/root/env femboyos:buildenv make build-x86_64');
}

async function upload() {
  await run('curl -H "Authorization: $ZIPLINE_TOKEN" https://i.ploszukiwacz.is-a.dev/api/upload -F file=dist/kernel.iso');
}

export default [makeDockerContainer, compile, upload];
