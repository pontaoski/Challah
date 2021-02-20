#!/usr/bin/env sh

rm -rf /tmp/challah-test-screenshots
mkdir -p /tmp/challah-test-screenshots
qbs build -p autotest-runner
