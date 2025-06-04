#!/usr/bin/env bash
# ---------------------------------------------------------------
# build.sh  —  кросс-платформенная сборка проекта QR-SLAM
#
#   ./scripts/build.sh                 # Release, auto-cores
#   ./scripts/build.sh -t Debug -j 8   # Debug, 8 потоков
#
# Опции
#   -t|--type   [Release|Debug|RelWithDebInfo]
#   -j|--jobs   N                 – параллельных потоков
#   -c|--clean                  – удалить каталог build/
#   -h|--help
#
# Только стандартный bash + cmake + make/ninja.
# ---------------------------------------------------------------
set -euo pipefail

# ---------- defaults ----------
BUILD_TYPE="Release"
JOBS=$(nproc || sysctl -n hw.ncpu || echo 4)
BUILD_DIR="build"
GENERATOR="Ninja"        # заменить на "Unix Makefiles", если не любите ninja

# ---------- parse args ----------
while [[ $# -gt 0 ]]; do
  case "$1" in
    -t|--type)   BUILD_TYPE="$2"; shift 2;;
    -j|--jobs)   JOBS="$2"; shift 2;;
    -c|--clean)  echo "[build] cleaning ${BUILD_DIR}"; rm -rf "${BUILD_DIR}"; exit 0;;
    -h|--help)
      echo "Usage: $0 [-t Release|Debug] [-j N] [-c]"
      exit 0;;
    *) echo "[build] unknown arg: $1"; exit 1;;
  esac
done

echo "[build] type=${BUILD_TYPE} | jobs=${JOBS}"

# ---------- configure ----------
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake .. \
  -G "${GENERATOR}" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# ---------- build ----------
if [[ "${GENERATOR}" == "Ninja" ]]; then
  ninja -j "${JOBS}"
else
  make -j "${JOBS}"
fi

echo -e "\n[build] done → ${BUILD_DIR}/"
