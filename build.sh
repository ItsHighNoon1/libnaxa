#!/bin/bash

VERSION="0.0.1"

# Allow compiler to be specified on command line (for Fil-C)
if [ -n "$1" ]; then
    CC="$1"
    echo "Specified compiler $1"
else
    CC="clang"
    echo "Fell back to compiler $CC"
fi

# Grab clang flags from compile_flags.txt for consistency with clangd
readarray -t flags < compile_flags.txt
echo "Using flags: $(IFS=$' '; echo "${flags[*]}")"

# Go find all C sources
source_files=()
while IFS= read -r line; do
    source_files+=("${line#src/}")
done < <(find "src" -type f -name "*.c")

# Compile all C files to ./obj
mkdir -p obj/lib
object_files=()
for source in "${source_files[@]}"; do
    object="obj/lib/${source%.*}.o"
    object_files+=("$object")
    # Going to roll with full builds unless things get bad
    # if [[ "$object" -ot "src/$source" ]]; then
        echo "Building $source"
        dir="${object%/*}"
        mkdir -p $dir
        $CC -c -o "$object" "src/$source" $(IFS=$'\n'; echo "${flags[*]}") &
    # fi
done
wait

# Link
echo "Linking"
mkdir -p lib
$CC $(IFS=$'\n'; echo "${flags[*]}") -shared -o "lib/libnaxa.so.$VERSION" $(IFS=$'\n'; echo "${object_files[*]}")
ln -nsf libnaxa.so.$VERSION lib/libnaxa.so

echo "Done building library"

# Compile the test program
mkdir -p obj/test
source_files=()
while IFS= read -r line; do
    source_files+=("${line#test/src/}")
done < <(find "test/src" -type f -name "*.c")
object_files=()
for source in "${source_files[@]}"; do
    object="obj/test/src/${source%.*}.o"
    object_files+=("$object")
    echo "Building $source"
    dir="${object%/*}"
    mkdir -p $dir
    $CC -c -o "$object" "test/src/$source" $(IFS=$'\n'; echo "${flags[*]}") &
done
wait
mkdir -p bin
$CC -Llib -lnaxa $(IFS=$'\n'; echo "${flags[*]}") -o "bin/test" $(IFS=$'\n'; echo "${object_files[*]}")