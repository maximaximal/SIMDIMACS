#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo "Checking for equality between read and original file for all files in test"

for f in $SCRIPT_DIR/test/*.dimacs; do
    if ./driver "$f" | cmp $f; then
        echo "  ./driver $f"
    else
        echo "! ./driver $f"
    fi

    if ./driver -F "$f" | cmp $f; then
        echo "  ./driver -F $f"
    else
        echo "! ./driver -F $f"
    fi
done
