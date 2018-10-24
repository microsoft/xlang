# get current script directory in bash : https://stackoverflow.com/a/246128 
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

SRC_ROOT_PATH="$(dirname "$SCRIPT_DIR")"
BUILD_PATH="$(dirname "$SRC_ROOT_PATH")"/_build/Ubuntu_18.04/Debug

cmake "${SRC_ROOT_PATH}" "-B${BUILD_PATH}" -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang
ninja -C "${BUILD_PATH}"