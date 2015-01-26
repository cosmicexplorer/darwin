#!/bin/bash

mkdir -p ./doc

cldoc generate -std=c11 $(pkg-config --cflags glib-2.0) -- \
    --output ./doc --language c --basedir . src/*

