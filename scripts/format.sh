#!/bin/bash

cd "$(dirname "$0")"/..
pwd
find_sources="find include src tests examples -type f -name *\.h -o -name *\.cpp"

echo -n "Running clang-format "

$find_sources | xargs -I {} sh -c "clang-format -i {}; echo -n '.'"

