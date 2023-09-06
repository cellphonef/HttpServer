
SOURCE_DIR=$(pwd)
BUILD_DIR=${BUILD_DIR:-../build}
BUILD_TYPE=${BUILD_TYPE:-release}
INSTALL_DIR=${INSTALL_DIR:-../${BUILD_TYPE}}
CXX=${CXX:-g++}

mkdir -p ${BUILD_DIR}/${BUILD_TYPE}\
  && cd ${BUILD_DIR}/${BUILD_TYPE} \
  && cmake \
       -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
       -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
       ${SOURCE_DIR} \
  && make $*
