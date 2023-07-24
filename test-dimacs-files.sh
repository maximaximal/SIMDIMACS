#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo "Checking for equality between read and original file for all files in test"

for f in $SCRIPT_DIR/test/*.dimacs; do
    echo "  $f"
    if ! ./driver "$f" | diff /dev/stdin $f; then
        echo "! Detected difference with $f ! "
        echo "! Check with ./driver $f"
    fi
done
