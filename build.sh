#!/bin/bash

CORES=$(nproc)

BUILD_TESTS="OFF"
BUILD_EXAMPLES="OFF"
BUILD_PYSTRATEGY="OFF"

BUILD_ALL="OFF"

set -euo pipefail

usage(){
  cat <<EOF
Usage: $0 [options]

Options:
  -t, --tests           Build tests (requires gtest installed)
  -e, --examples        Build examples
  -p, --pystrategy      Build Python strategy module
  -h, --help            Show this help message and exit
EOF
  exit 0
}

while [[ $# -gt 0 ]]; do
  case $1 in
    -t|--tests)        BUILD_TESTS="ON";        shift ;;
    -e|--examples)     BUILD_EXAMPLES="ON";     shift ;;
    -p|--pystrategy)   BUILD_PYSTRATEGY="ON";   shift ;;
    #-a|--all)          BUILD_ALL="ON";          shift ;;
    -h|--help)      usage                     ;;
    *)              echo "Unknown option: $1" >&2; usage ;;
  esac
done

if [ -d "build" ]; then
  rm -rf "build"
fi

mkdir build && cd build

cmake .. -DBUILD_TESTS=$BUILD_TESTS \
-DBUILD_EXAMPLES="$BUILD_EXAMPLES" \
-DBUILD_PYSTRATEGY="$BUILD_PYSTRATEGY" \
-DBUILD_ALL="$BUILD_ALL" \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_CXX_COMPILER=/usr/bin/clang++
	
cmake --build . -j"$CORES"

if [ -d "python" ]; then
  cp -r ../python .
fi
	
shopt -s extglob
rm -rf -- !(bin|python)
