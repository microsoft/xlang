# get current script directory in bash : https://stackoverflow.com/a/246128 
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

# parse command line arguments with getopt : https://stackoverflow.com/a/29754866
set -o errexit -o pipefail -o noclobber -o nounset # saner programming env: these switches turn some bugs into errors

! getopt --test > /dev/null 
if [[ ${PIPESTATUS[0]} -ne 4 ]]; then
    echo "I’m sorry, `getopt --test` failed in this environment."
    exit 1
fi

OPTIONS=vfb:t:
LONGOPTS=verbose,force-cmake,build-type:,target:

# -use ! and PIPESTATUS to get exit code with errexit set
# -temporarily store output to be able to check for errors
# -activate quoting/enhanced mode (e.g. by writing out “--options”)
# -pass arguments only via   -- "$@"   to separate them correctly
! PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTS --name "$0" -- "$@")
if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
    # e.g. return value is 1
    #  then getopt has complained about wrong arguments to stdout
    exit 2
fi
# read getopt’s output this way to handle the quoting right:
eval set -- "$PARSED"

force=false verbose="" buildType=Debug target=""
# now enjoy the options in order and nicely split until we see --
while true; do
    case "$1" in
        -f|--force-cmake)
            force=true
            shift
            ;;
        -v|--verbose)
            verbose="-v"
            shift
            ;;
        -b|--build-type)
            buildType="$2"
            shift 2
            ;;
        -t|--target)
            target="$2"
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Programming error"
            exit 3
            ;;
    esac
done

SRC_ROOT_PATH="$(dirname "$SCRIPT_DIR")"
BUILD_PATH="$(dirname "$SRC_ROOT_PATH")"/_build/Ubuntu_18.04/"$buildType/"

if $force || ! [ -e "$BUILD_PATH/CMakeCache.txt" ]
then
  cmake $SRC_ROOT_PATH "-B$BUILD_PATH" -GNinja "-DCMAKE_BUILD_TYPE=$buildType" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang
fi

ninja -C $BUILD_PATH $verbose $target