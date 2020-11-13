#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SOURCE_DIR=$(dirname "${SCRIPT_DIR}")
PO_DIR=${SCRIPT_DIR}/po

# Go to the source root directory to write correct relative paths to the pot-file.
cd "${SOURCE_DIR}" || exit 1
mkdir -p "$PO_DIR"

lupdate "${SCRIPT_DIR}"/resources/data.qrc -ts "$PO_DIR"/Murmur_*.ts
