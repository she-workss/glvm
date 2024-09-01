#!/bin/sh

# set -xe

# -d - Debug
# -r - Release
compile_mode_flag=$1

if [ "${compile_mode_flag}" = "-d" ]; then
    compile_mode=debug
elif [ "${compile_mode_flag}" = "-r" ]; then
    compile_mode=release
else
    echo [ERROR] Invalid compile mode.
    echo [ERROR] Use -d for debug or -r for release.
    exit 1
fi

project_dir=$(pwd)
build_folder=build

echo [INFO] Compiling in ${compile_mode} mode...
cmake --no-warn-unused-cli \
      -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
      -DCMAKE_BUILD_TYPE:STRING=${compile_mode} \
      -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc \
      -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++ \
      "-S${project_dir}" \
      "-B${project_dir}/${build_folder}/${compile_mode}" \
      -G "Unix Makefiles"
cmake --build "${project_dir}/${build_folder}/${compile_mode}" \
      --config ${compile_mode} \
      --target all \
      -j 8

sudo rmdir bin > /dev/null 2>&1
