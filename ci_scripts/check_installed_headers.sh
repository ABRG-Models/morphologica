#!/bin/bash
morph_headers=$(find ../morph | grep -v "fonts" | rev | cut -d"/" -f1 | rev | grep "\.h")
installed_headers=$(grep "\.h" install_manifest.txt | rev | cut -d"/" -f1 | rev)
missing_headers=$(echo $morph_headers | tr ' ' '\n' | grep -Fxv -f <(echo $installed_headers | tr ' ' '\n'))

if [[ -z "$missing_headers" ]]; then
    echo "All headers from the morph directory have been installed successfully."
    exit 0
else
    echo "The following headers are in the morph directory, but were not installed:"
    echo "$missing_headers"
    exit 1
fi
