#!/usr/bin/env bash
set -euo pipefail
TAG=${1:-}

export PLATFORMIO_BUILD_FLAGS="'-DVERSION_NUMBER=\"${TAG}\"'"
pio run -t upload