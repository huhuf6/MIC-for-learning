#!/usr/bin/env bash
set -euo pipefail

# ONNX -> MLIR 转换脚本
# 用法:
#   onnx_to_mlir.sh <input.onnx> <output.mlir>
#
# 设计目标:
# 1) 对外暴露固定的两参数接口，适配 mlir2trt --onnx-mlir-converter
# 2) 兼容不同 onnx-mlir 版本的 EmitMLIR 参数风格
# 3) 给出明确错误信息，便于排查环境问题

if [[ $# -ne 2 ]]; then
  echo "用法: $0 <input.onnx> <output.mlir>" >&2
  exit 2
fi

INPUT_ONNX="$1"
OUTPUT_MLIR="$2"
ONNX_MLIR_BIN="${ONNX_MLIR_BIN:-onnx-mlir}"

if [[ ! -f "$INPUT_ONNX" ]]; then
  echo "[onnx_to_mlir] 输入文件不存在: $INPUT_ONNX" >&2
  exit 2
fi

if ! command -v "$ONNX_MLIR_BIN" >/dev/null 2>&1; then
  echo "[onnx_to_mlir] 未找到转换器: $ONNX_MLIR_BIN" >&2
  echo "[onnx_to_mlir] 请安装 onnx-mlir，或设置 ONNX_MLIR_BIN 指向可执行文件" >&2
  exit 127
fi

OUT_DIR="$(dirname "$OUTPUT_MLIR")"
mkdir -p "$OUT_DIR"

TMP_BASE="/tmp/onnx_to_mlir_$$"
TMP_LOG="${TMP_BASE}.log"
trap 'rm -f "${TMP_BASE}" "${TMP_BASE}.mlir" "${TMP_BASE}.onnx.mlir" "$TMP_LOG"' EXIT

run_convert() {
  local mode="$1"
  case "$mode" in
    emit_short)
      "$ONNX_MLIR_BIN" "$INPUT_ONNX" -EmitMLIR -o "$TMP_BASE" >"$TMP_LOG" 2>&1
      ;;
    emit_long)
      "$ONNX_MLIR_BIN" "$INPUT_ONNX" --EmitMLIR -o "$TMP_BASE" >"$TMP_LOG" 2>&1
      ;;
    *)
      return 2
      ;;
  esac
}

SUCCESS=0
if run_convert emit_short; then
  SUCCESS=1
elif run_convert emit_long; then
  SUCCESS=1
fi

if [[ "$SUCCESS" -ne 1 ]]; then
  echo "[onnx_to_mlir] onnx-mlir 转换失败。" >&2
  echo "[onnx_to_mlir] 尝试命令: -EmitMLIR / --EmitMLIR 均失败。" >&2
  echo "[onnx_to_mlir] 日志如下:" >&2
  cat "$TMP_LOG" >&2 || true
  exit 1
fi

if [[ -f "${TMP_BASE}.mlir" ]]; then
  cp "${TMP_BASE}.mlir" "$OUTPUT_MLIR"
elif [[ -f "${TMP_BASE}.onnx.mlir" ]]; then
  cp "${TMP_BASE}.onnx.mlir" "$OUTPUT_MLIR"
elif [[ -f "${TMP_BASE}" ]]; then
  cp "${TMP_BASE}" "$OUTPUT_MLIR"
else
  echo "[onnx_to_mlir] 转换成功但未找到输出文件。" >&2
  echo "[onnx_to_mlir] 期望文件之一: ${TMP_BASE}.mlir / ${TMP_BASE}.onnx.mlir / ${TMP_BASE}" >&2
  echo "[onnx_to_mlir] 转换日志如下:" >&2
  cat "$TMP_LOG" >&2 || true
  exit 1
fi

echo "[onnx_to_mlir] 转换完成: $INPUT_ONNX -> $OUTPUT_MLIR"
