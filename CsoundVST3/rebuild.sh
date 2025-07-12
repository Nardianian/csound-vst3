#!/bin/bash
rm -rf build
cmake -B build -S .
cmake --build build --config Release
