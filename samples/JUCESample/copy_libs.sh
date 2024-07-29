#!/bin/sh

LIB_PATH="../../../../bin/shared/Release"
BUILD_PATH="build/Release"
cp ${LIB_PATH}/lib*.dylib* ${BUILD_PATH}


LIB_PATH="../../../../bin/shared/Debug"
BUILD_PATH="build/Debug"
cp ${LIB_PATH}/lib*.dylib* ${BUILD_PATH}
exit 0
