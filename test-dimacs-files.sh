#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo "Checking for equality between read and original file for all files in test"

BIN="./driver"

if [[ "$#" -eq 1 ]]; then
    BIN=$1
    echo "Using call $BIN to run."
fi


for f in $SCRIPT_DIR/test/*.dimacs $SCRIPT_DIR/test/*.qdimacs $SCRIPT_DIR/test/*.lrat $SCRIPT_DIR/test/*.drat; do
    if $BIN "$f" | cmp $f; then
        echo "  $BIN $f"
    else
        echo "! $BIN $f"
    fi
done
