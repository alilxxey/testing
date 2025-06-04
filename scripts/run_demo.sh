#!/usr/bin/env bash
# ---------------------------------------------------------------
# run_demo.sh  —  запуск QR-SLAM демо
#
#   ./scripts/run_demo.sh                       # дефолты
#   ./scripts/run_demo.sh -c cfg.yaml -v vocab.fbow -d 1
#
# Опции
#   -c|--config   path/to/config.yaml
#   -v|--vocab    path/to/orb_vocab.fbow
#   -d|--cam      camera_id     (default 0)
#   -h|--help
# ---------------------------------------------------------------
set -euo pipefail

# ---------- defaults ----------
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="${ROOT_DIR}/build/qr_slam_demo"
CONFIG="${ROOT_DIR}/config.yaml"
VOCAB="${ROOT_DIR}/orb_vocab.fbow"
CAM_ID=0

# ---------- parse args ----------
while [[ $# -gt 0 ]]; do
  case "$1" in
    -c|--config) CONFIG="$2"; shift 2;;
    -v|--vocab)  VOCAB="$2"; shift 2;;
    -d|--cam)    CAM_ID="$2"; shift 2;;
    -h|--help)
      echo "Usage: $0 [-c config.yaml] [-v vocab.fbow] [-d cam_id]"
      exit 0;;
    *) echo "[run] unknown arg: $1"; exit 1;;
  esac
done

# ---------- sanity checks ----------
[[ -x "${BIN}"      ]] || { echo "[run] binary not found: ${BIN}"; exit 1; }
[[ -f "${CONFIG}"   ]] || { echo "[run] config not found: ${CONFIG}"; exit 1; }
[[ -f "${VOCAB}"    ]] || { echo "[run] vocab  not found: ${VOCAB}"; exit 1; }

# ---------- LD_LIBRARY_PATH helper (Linux) ----------
if [[ "$(uname)" == "Linux" ]]; then
  export LD_LIBRARY_PATH="/usr/local/lib:${LD_LIBRARY_PATH:-}"
fi

echo "[run] binary : ${BIN}"
echo "[run] config : ${CONFIG}"
echo "[run] vocab  : ${VOCAB}"
echo "[run] camera : ${CAM_ID}"

"${BIN}" \
  --config "${CONFIG}" \
  --vocab  "${VOCAB}" \
  --cam    "${CAM_ID}"
