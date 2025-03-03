#!/bin/bash

failed_examples=()

pushd "$(dirname "$0")"

for example_dir in */; do
    pushd "$example_dir"
    ln -s ../../../morphologica
    mkdir build
    pushd build
    cmake ..
    make
    if [ "$?" -ne '0'  ]; then
        failed_examples+=( "$example_dir" )
    fi
    popd > /dev/null # build
    popd > /dev/null # example_dir
done

echo
if [ ${#failed_examples[@]} -eq 0 ]; then
    echo "All standalone examples have been built successfully !"
    exit 0
else
    for i in ${failed_examples[@]}; do
        echo -e "Standalone example $i has failed..."
    done
    exit 1
fi

popd > /dev/null # standalone_examples
