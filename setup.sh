#!/usr/bin/env bash
set -euo pipefail

# Install dependencies for MIC-for-learning on Ubuntu 22.04+.
# Usage:
#   bash setup.sh

if [[ "${EUID}" -ne 0 ]]; then
  SUDO="sudo"
else
  SUDO=""
fi

echo "[1/5] Refresh apt package index..."
$SUDO apt-get update

echo "[2/5] Install base build dependencies..."
$SUDO apt-get install -y \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  clang \
  python3-pip \
  python3-onnx \
  python3-torch \
  python3-pycuda \
  llvm-15 \
  llvm-15-dev \
  libmlir-15-dev \
  mlir-15-tools \
  libonnx-dev \
  libnvinfer-dev \
  libnvinfer-plugin-dev \
  libnvonnxparsers-dev \
  tensorrt \
  tensorrt-dev \
  nvidia-cuda-toolkit

echo "[3/5] Check critical tools..."
command -v cmake >/dev/null
command -v nvcc >/dev/null
command -v mlir-tblgen >/dev/null || true

echo "[4/5] Print detected versions..."
cmake --version | head -n 1
nvcc --version | sed -n '1,3p'
python3 -V

echo "[5/5] Done."
cat <<'EOF'
Next suggested configure/build command:

cmake -S . -B build \
  -DCMAKE_CUDA_COMPILER=/usr/bin/nvcc \
  -DCUDA_TOOLKIT_ROOT_DIR=/usr \
  -DLLVM_DIR=/usr/lib/llvm-15/lib/cmake/llvm \
  -DMLIR_DIR=/usr/lib/llvm-15/lib/cmake/mlir \
  -DTENSORRT_LIBRARY=/usr/lib/x86_64-linux-gnu/libnvinfer.so \
  -DTENSORRT_PLUGIN_LIBRARY=/usr/lib/x86_64-linux-gnu/libnvinfer_plugin.so \
  -DTENSORRT_ONNX_PARSER_LIBRARY=/usr/lib/x86_64-linux-gnu/libnvonnxparser.so \
  -DTENSORRT_PARSERS_LIBRARY=/usr/lib/x86_64-linux-gnu/libnvonnxparser.so

cmake --build build -j"$(nproc)"

Note:
- Current source may still fail to build if your MLIR API version differs from
  what this project expects.
- The project hardcodes TensorRT_ROOT to /usr/local/TensorRT-8.5.2.2 in
  CMakeLists.txt, so explicit -DTENSORRT_* flags are used as a workaround.
EOF
