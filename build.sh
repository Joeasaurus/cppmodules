#!/usr/bin/env bash

mkdir -p cmko build
rm -rf cmko/* build/*

pushd cmko

cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=debug -DDEFAULT_MODULES=yes .. && \
make -j 4

if [[ $? -eq 0 ]]; then
    popd

    pushd build

    lldb example

    popd
fi
