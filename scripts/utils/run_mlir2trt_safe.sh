#!/usr/bin/env bash
set -euo pipefail

# 可靠运行 mlir2trt：若进程因 SIGSEGV 退出(139)，自动重试。
#
# 用法：
#   run_mlir2trt_safe.sh [--max-retries N] -- <mlir2trt args...>
# 示例：
#   scripts/utils/run_mlir2trt_safe.sh --max-retries 8 -- --onnx-import=trt test_model.onnx

MAX_RETRIES=5

while [[ $# -gt 0 ]]; do
  case "$1" in
    --max-retries)
      if [[ $# -lt 2 ]]; then
        echo "缺少 --max-retries 参数值" >&2
        exit 2
      fi
      MAX_RETRIES="$2"
      shift 2
      ;;
    --)
      shift
      break
      ;;
    *)
      echo "未知参数: $1" >&2
      echo "用法: $0 [--max-retries N] -- <mlir2trt args...>" >&2
      exit 2
      ;;
  esac
done

if [[ $# -eq 0 ]]; then
  echo "未提供 mlir2trt 参数" >&2
  echo "用法: $0 [--max-retries N] -- <mlir2trt args...>" >&2
  exit 2
fi

CMD=(build/tools/mlir2trt/mlir2trt "$@")

attempt=1
while (( attempt <= MAX_RETRIES )); do
  echo "[safe-run] attempt ${attempt}/${MAX_RETRIES}: ${CMD[*]}"
  set +e
  "${CMD[@]}"
  rc=$?
  set -e

  if [[ $rc -eq 0 ]]; then
    echo "[safe-run] 成功"
    exit 0
  fi

  if [[ $rc -eq 139 ]]; then
    echo "[safe-run] 检测到 SIGSEGV(139)，准备重试..."
    attempt=$((attempt + 1))
    sleep 1
    continue
  fi

  echo "[safe-run] 非 SIGSEGV 错误，停止重试。exit=${rc}" >&2
  exit "$rc"
done

echo "[safe-run] 达到最大重试次数，仍未成功。" >&2
exit 139
