#!/bin/bash

# * variables

# Color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
PURPLE='\033[0;35m'
RESET='\033[0m' # No Color
readonly GREEN RED YELLOW CYAN PURPLE RESET

# * variables end

# * parsing arguments
build_type="Debug"
if [[ "$1" == "--release" ]]; then
    build_type="Release"
fi
# * parsing arguments end

clear
clear
clear

set -e

# setting preset and overriding build type
echo -e "${YELLOW}Generating build files using CMake preset ... ${RESET}"
cmake --preset default-configure -DCMAKE_BUILD_TYPE=$build_type

echo -e "${YELLOW}Compiling using CMake preset ... ${RESET}"
cmake --build --preset default-build --config $build_type

start=$(date +%s.%N)
echo -e "${PURPLE}Executable Started ... ${RESET}"
if [[ "$OSTYPE" == "darwin"* ]]; then
    build/main
elif [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "cygwin"* || "$OSTYPE" == "win"* ]]; then
    build/$build_type/main.exe
else
    echo -e "${RED}Unsupported OS type: $OSTYPE${RESET}"
    exit 1
fi

end=$(date +%s.%N)

# * print time

elapsed=$(awk -v e="$end" -v s="$start" 'BEGIN { printf "%.9f", e - s }')

if awk -v e="$elapsed" 'BEGIN { exit !(e <= 1) }'; then
    # less than 1 sec
    printf "${PURPLE}Elapsed Time: %.3f milliseconds${RESET}\n" "$(awk -v t="$elapsed" 'BEGIN { printf "%.3f", t * 1000 }')"
else
    # less than 1 sec
    printf "${PURPLE}Elapsed Time: %.3f seconds${RESET}\n" "$elapsed"
fi

# * print time end
