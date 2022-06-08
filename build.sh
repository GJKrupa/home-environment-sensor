#!/usr/bin/env bash
set -euo pipefail
REPO=${1:-}
TAG=${2:-}

export PLATFORMIO_BUILD_FLAGS="'-DVERSION_NUMBER=\"${TAG}\"'"
pio run

docker build \
    --progress=plain \
    --build-arg VERSION_NUMBER="${TAG}" \
    -t "${REPO}:${TAG}" \
    .