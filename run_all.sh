#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${ROOT_DIR}"

retry() {
  local tries="$1"
  shift
  local n=1
  while true; do
    if "$@"; then
      return 0
    fi
    if [[ "${n}" -ge "${tries}" ]]; then
      echo "[ERROR] Command failed after ${tries} attempts: $*" >&2
      return 1
    fi
    n=$((n + 1))
    echo "[WARN] Retry ${n}/${tries}: $*"
    sleep 3
  done
}

echo "[1/5] Checking toolchain..."
command -v cmake >/dev/null
command -v python3 >/dev/null
command -v nvcc >/dev/null

echo "[2/5] Installing Python runtime (CUDA PyTorch nightly + ONNX + CUDA libs)..."
export PIP_DEFAULT_TIMEOUT=1000
export PATH="$HOME/.local/bin:$PATH"

retry 3 python3 -m pip install --user --upgrade --pre --no-deps \
  torch torchvision torchaudio \
  --index-url https://download.pytorch.org/whl/nightly/cu128

retry 3 python3 -m pip install --user --upgrade \
  onnx==1.20.1 onnxscript==0.6.2 "numpy<2"

retry 3 python3 -m pip install --user --upgrade \
  "cuda-toolkit[cublas,cudart,cufft,cufile,cupti,curand,cusolver,cusparse,nvjitlink,nvrtc,nvtx]==12.8.1" \
  -i https://pypi.nvidia.com

retry 3 python3 -m pip install --user --upgrade \
  nvidia-cusparselt-cu12==0.7.1 \
  nvidia-nvshmem-cu12==3.4.5 \
  nvidia-nccl-cu12==2.29.3 \
  -i https://pypi.nvidia.com

echo "[3/5] Configuring CMake..."
cmake -S . -B build \
  -DCMAKE_CUDA_COMPILER=/usr/bin/nvcc \
  -DCUDA_TOOLKIT_ROOT_DIR=/usr \
  -DLLVM_DIR=/usr/lib/llvm-15/lib/cmake/llvm \
  -DMLIR_DIR=/usr/lib/llvm-15/lib/cmake/mlir \
  -DTENSORRT_LIBRARY=/usr/lib/x86_64-linux-gnu/libnvinfer.so \
  -DTENSORRT_PLUGIN_LIBRARY=/usr/lib/x86_64-linux-gnu/libnvinfer_plugin.so \
  -DTENSORRT_ONNX_PARSER_LIBRARY=/usr/lib/x86_64-linux-gnu/libnvonnxparser.so \
  -DTENSORRT_PARSERS_LIBRARY=/usr/lib/x86_64-linux-gnu/libnvonnxparser.so

echo "[4/5] Building..."
cmake --build build --target mlir2trt -j"$(nproc)"

echo "[5/5] Running pipeline test..."
PATH="${ROOT_DIR}/build/tools/mlir2trt:${PATH}" python3 scripts/utils/test_pipeline.py

echo "[DONE] Environment install + build + test completed."
