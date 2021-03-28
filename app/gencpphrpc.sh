#!/usr/bin/env bash

mkdir -p "gen"

for dir in $(find "protocol" -name '*.proto' -print0 | xargs -0 -n1 dirname | sort | uniq); do
    echo "Generating files in ${dir}..."
    find "${dir}" -name '*.proto'

    protoc \
    --proto_path=protocol \
    --cpp_out=./gen \
    --hrpc_out=./gen \
    --hrpc_opt="qt_cpp_client" \
    $(find "${dir}" -name '*.proto')
done
