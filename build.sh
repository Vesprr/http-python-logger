#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
PURPLE='\033[0;35m'
RESET='\033[0m'
readonly GREEN RED YELLOW CYAN PURPLE RESET

build_type="Debug"
if [[ "$1" == "--release" ]]; then
    build_type="Release"
fi

clear
clear
clear

set -e

echo -e "${YELLOW}Generating static library build files...${RESET}"
cmake --preset default-configure \
    -DCMAKE_BUILD_TYPE=$build_type \
    -DBUILD_STATIC_LIB=ON

echo -e "${YELLOW}Building static library...${RESET}"
cmake --build --preset default-build --config $build_type

echo -e "${GREEN}Build complete.${RESET}"

# get cmake project name
project_name=$(grep "^CMAKE_PROJECT_NAME:" build/CMakeCache.txt | cut -d'=' -f2)

if [[ -f "build/$build_type/${project_name}.a" ]]; then
    echo -e "${CYAN}Static library generated: build/$build_type/${project_name}.a${RESET}"
elif [[ -f "build/${project_name}.a" ]]; then
    echo -e "${CYAN}Static library generated: build/${project_name}.a${RESET}"
else
    echo -e "${RED}Could not find output library.${RESET}"
fi
