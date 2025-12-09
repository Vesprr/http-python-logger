#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
PURPLE='\033[0;35m'
RESET='\033[0m'
readonly GREEN RED YELLOW CYAN PURPLE RESET

FILES_DIR="embedded/files"
OUT_DIR="embedded/incs"
EXCLUDE_SUFFIXES=("~" ".bak" ".tmp" ".md")

should_exclude() {
    local file="$1"
    for suffix in "${EXCLUDE_SUFFIXES[@]}"; do
        if [[ "$file" == *"$suffix" ]]; then
            return 0
        fi
    done
    return 1
}

clear
clear
clear
set -e

mkdir -p "$OUT_DIR"

start=$(date +%s.%N)
echo -e "${YELLOW}Embedding files from $FILES_DIR to $OUT_DIR ...${RESET}"

# loop through all files in FILES_DIR
for f in "$FILES_DIR"/*; do
    [ -f "$f" ] || continue

    if should_exclude "$f"; then
        echo -e "  ${RED}-> Excluding File: $f${RESET}"
        continue
    else
        echo -e "  ${GREEN}-> Found File: $f${RESET}"
    fi

    base=$(basename "$f")
    name="${base//[^a-zA-Z0-9_]/_}"
    out="$OUT_DIR/${base}.inc"

    echo -e "    ${CYAN}=> Embedding $f -> $out${RESET}"

    # Generate C array with xxd
    xxd -i -n "$name" "$f" |
        sed -E 's/^unsigned char/const unsigned char/; s/^unsigned int/const unsigned int/' \
            >"$out"
done

end=$(date +%s.%N)
elapsed=$(awk -v e="$end" -v s="$start" 'BEGIN { printf "%.9f", e - s }')

if awk -v e="$elapsed" 'BEGIN { exit !(e <= 1) }'; then
    printf "${PURPLE}Elapsed Time: %.3f milliseconds${RESET}\n" "$(awk -v t="$elapsed" 'BEGIN { printf "%.3f", t * 1000 }')"
else
    printf "${PURPLE}Elapsed Time: %.3f seconds${RESET}\n" "$elapsed"
fi

echo -e "${GREEN}Embedding complete.${RESET}"
