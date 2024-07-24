#!/bin/sh

LIB_PATH="../../../../../bin/static/Release"
BUILD_PATH="build/Release"
cp ${LIB_PATH}/lib*.dylib* ${BUILD_PATH}


LIB_PATH="../../../../../bin/static/Debug"
BUILD_PATH="build/Debug"
cp ${LIB_PATH}/lib*.dylib* ${BUILD_PATH}
exit 0
