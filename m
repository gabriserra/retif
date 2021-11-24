#!/bin/bash

# -------------------------------------------------------- #

function jump_and_print_path() {
    cd -P "$(dirname "$1")" >/dev/null 2>&1 && pwd
}

function get_script_path() {
    local _SOURCE
    local _PATH

    _SOURCE="${BASH_SOURCE[0]}"

    # Resolve $_SOURCE until the file is no longer a symlink
    while [ -h "$_SOURCE" ]; do
        _PATH="$(jump_and_print_path "${_SOURCE}")"
        _SOURCE="$(readlink "${_SOURCE}")"

        # If $_SOURCE is a relative symlink, we need to
        # resolve it relative to the path where the symlink
        # file was located
        [[ $_SOURCE != /* ]] && _SOURCE="${_PATH}/${_SOURCE}"
    done

    _PATH="$(jump_and_print_path "$_SOURCE")"
    echo "${_PATH}"
}

# Argument: relative path of project directory wrt this
# script directory
function get_project_path() {
    local _PATH
    local _PROJPATH

    _PATH=$(get_script_path)
    _PROJPATH=$(realpath "${_PATH}/$1")
    echo "${_PROJPATH}"
}

# ======================================================== #
# ---------------------- ARGUMENTS ----------------------- #
# ======================================================== #

function toshortopts() {
    local out=
    newargs=()
    while [ $# -gt 0 ]; do
        case "$1" in
        --help)         printf -v out '%s' '-h' ;;
        --verbose)      printf -v out '%s' '-v' ;;
        --deb)          printf -v out '%s' '-d' ;;
        --rpm)          printf -v out '%s' '-r' ;;
        --jobs)         printf -v out '%s' '-j' ;;
        --parallel)     printf -v out '%s' '-J' ;;
        --generator)    printf -v out '%s' '-G' ;;
        --build-type)   printf -v out '%s' '-b' ;;
        --build-path)   printf -v out '%s' '-p' ;;
        *)              printf -v out '%s' "$1" ;;
        esac
        shift

        newargs+=( "$out" )
    done
}

function separate_args() {
    local optstring="$1"
    local OPTION
    shift

    opt_args=()
    pos_args=()

    while [ $# -gt 0 ]; do
        unset OPTIND
        unset OPTARG
        unset OPTION
        while getopts ":$optstring" OPTION; do
            if [ "$OPTION" != : ]; then
                opt_args+=("-$OPTION")
            else
                OPTARG="-$OPTARG"
            fi

            if [ ! -z "$OPTARG" ]; then
                opt_args+=("$OPTARG")
            fi

            unset OPTARG
        done

        shift $((OPTIND - 1)) || true
        pos_args+=("$1")
        shift || true
    done
}

function missing_argument() {
    printf 'Error: option %s requires an argument!\n\n' "$@" >&2
    usage
}

function unsupported_argument() {
    printf 'Error: option %s does not support argument %s!\n\n' "$@" >&2
    usage
}

function unrecognized() {
    printf 'Error: unrecognized %s %s!\n\n' "$1" "${*:2}" >&2
}

function parse_opt_args() {
    local optstring="$1"
    local OPTION
    shift

    unset OPTIND
    unset OPTARG
    unset OPTION
    while getopts "$optstring" OPTION; do
        case $OPTION in
        h)
            pos_args=()
            return 0
            ;;
        v)
            verbose=ON
            ;;
        d)
            package_deb=ON
            ;;
        r)
            package_rpm=ON
            ;;
        j)
            if [ -z "$OPTARG" ]; then
                missing_argument '-j|--jobs'
                return 1
            fi
            ;;
        J)
            jobs=""
            ;;
        b)
            if [ -z "$OPTARG" ]; then
                missing_argument '-b|--build-type'
                return 1
            fi

            case "$OPTARG" in
            debug) build_type='Debug' ;;
            release) build_type='Release' ;;
            release-wdebug) build_type='RelWithDebInfo' ;;
            *)
                unsupported_argument '-b|--build-type' "$OPTARG"
                return 1
                ;;
            esac
            ;;
        p)
            if [ -z "$OPTARG" ]; then
                missing_argument '-p|--build-path'
                return 1
            fi
            path_build="$OPTARG"
            ;;
        G)
            if [ -z "$OPTARG" ]; then
                missing_argument '-G|--generator'
                return 1
            fi
            generator="$OPTARG"
            ;;
        *)
            shift $((OPTIND - 1)) || true
            unrecognized 'option' "$1"
            return 1
            ;;
        esac
        unset OPTARG
    done

    # Too many options/unrecognized options
    shift $((OPTIND - 1)) || true
    if [ "$#" -gt 0 ]; then
        unrecognized 'options' "$@"
        return 1
    fi
}

function parse_pos_args() {
    commands=("$@")

    if [ $# -lt 1 ]; then
        commands=(usage)
    fi
}

# ======================================================== #
# ----------------------- COMMANDS ----------------------- #
# ======================================================== #

function usage() {
    cat <<EOF
usage: $0 [options] COMMAND [...COMMANDS]

Runs the specified list of commands using the given arguments

List of options (all optional):
  -h, --help        Prints this help message and returns
  -v, --verbose     Prints more info during execution
  -d, --deb         Enables the generation of the deb package
  -r, --rpm         Enables the generation of the rpm package
  -G, --generator GEN
                    Uses the provided CMake generator to build the project
  -J, --parallel    Enables parallel build execution with a default number of
                    processes
  -j, --jobs JOBS
                    Enables parallel build execution with JOBS processes
  -b, --build-type TARGET[=release*|debug|release-wdebug]
                    Specifies which version of the project to build
  -p, --build-path BUILDPATH[=build]
                    Specifies which path to use to build the project

List of commands:
    build           (Re-)Build the project
    clean           Clean the project build directory
    configure       (Re-)Configures the project
    install         (Re-)Install the project
    package         Generates the desired packages
    uninstall       Removes the installed files from paths
    usage           Prints this help message and returns
EOF
# TODO(gabrieleara): test
}

function reset_ran() {
    ran_build=0
    ran_clean=0
    ran_configure=0
    ran_install=0
    ran_package=0
    ran_uninstall=0
    ran_usage=0
}

function build() {
    if [ "$ran_build" = 1 ]; then
        return 0
    fi

    configure
    cmake --build "$path_build" --parallel $jobs
    ran_build=1
}

function clean() {
    rm -rf "$path_build"
    reset_ran
}

function configure() {
    if [ "$ran_configure" = 1 ]; then
        return 0
    fi

    if [ "$generator" = 'Ninja' ] && ! command -v ninja &> /dev/null
    then
        # Fallback to Unix Makefiles on Linux
        generator='Unix Makefiles'
    fi

    cmake -S "$path_src" -B "$path_build" \
        -G "$generator" \
        -DCMAKE_BUILD_TYPE="$build_type" \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL="$verbose" \
        -DCPACK_ENABLE_DEB="$package_deb" \
        -DCPACK_ENABLE_RPM="$package_rpm"

    ran_configure=1
}

function install() {
    if [ "$ran_install" = 1 ]; then
        return 0
    fi

    build
    sudo cmake --build "$path_build" --target install
    ran_install=1
}

function uninstall() {
    if [ "$ran_uninstall" = 1 ]; then
        return 0
    fi

    # This is a bit counter-intuitive, to uninstall we need to install first!
    # What we want actually is the install manifest to be present in the build
    # path!
    local install_manifest="${path_build}/install_manifest.txt"
    if [ ! -f "$install_manifest" ]; then
        install
    fi

    echo "Removing the following files:"
    cat "$install_manifest"
    echo ""
    cat "$install_manifest" | sudo xargs rm -f
    ran_uninstall=1
}

function package() {
    if [ "$ran_package" = 1 ]; then
        return 0
    fi

    configure
    build

    (
        set -e
        cd "$path_build"
        sudo cpack
        sudo chown -R $USER:$USER .
    )

    if [ "$package_deb" = 'ON' ]; then
        # TODO: sign the deb package
        :
    fi

    ran_package=1
}

(
    set -e

    # Variables and default values
    path_proj="$(get_project_path ".")"
    path_src="$path_proj"
    path_build="$path_proj/build"
    build_type="Release"
    package_deb=OFF
    package_rpm=OFF
    verbose=OFF
    jobs=1
    generator=Ninja

    commands=()

    opt_args=()
    pos_args=()
    optstring='hvJj:drb:p:G:'

    OPTERR=0

    # Separate optional from positional arguments, then parse them
    toshortopts "$@"

    separate_args   "$optstring" "${newargs[@]}"
    parse_opt_args  "$optstring" "${opt_args[@]}"
    parse_pos_args  "${pos_args[@]}"

    reset_ran

    for c in "${commands[@]}"; do
        # Trim the variable
        c="$(echo $c | xargs)"
        case "$c" in
        build) build ;;
        clean) clean ;;
        configure) configure ;;
        install) install ;;
        package) package ;;
        uninstall) uninstall ;;
        usage) usage ;;
        *)
            unrecognized 'command' "$c"
            usage
            false
            ;;
        esac
    done
)
