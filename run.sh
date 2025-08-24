#!/bin/bash

# variables

# Color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
PURPLE='\033[0;35m'
RESET='\033[0m' # No Color
readonly GREEN RED YELLOW CYAN PURPLE RESET

# variables end

# parsing arguments
build_type="Debug"
if [[ "$1" == "--release" ]]; then
    build_type="Release"
fi
# parsing arguments end

clear
clear
clear

set -e

mkdir -p build

echo -e "${YELLOW}Generate build files CMake ... ${RESET}"
cmake -B build -S . \
    -DCMAKE_TOOLCHAIN_FILE=$(pwd)/external/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=arm64-osx \
    -DCMAKE_BUILD_TYPE=$build_type

echo -e "${YELLOW}Compiling using CMake ... ${RESET}"
cmake --build build --config Release

start=$(date +%s.%N)
echo -e "${PURPLE}Executable Started ... ${RESET}"
build/main
end=$(date +%s.%N)

elapsed=$(awk -v e="$end" -v s="$start" 'BEGIN { printf "%.9f", e - s }')

if awk -v e="$elapsed" 'BEGIN { exit !(e <= 1) }'; then
    # less than 1 sec
    printf "${PURPLE}Elapsed Time: %.3f milliseconds${NC}\n" "$(awk -v t="$elapsed" 'BEGIN { printf "%.3f", t * 1000 }')"
else
    # less than 1 sec
    printf "${PURPLE}Elapsed Time: %.3f seconds${RESET}\n" "$elapsed"
fi
