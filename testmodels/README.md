# testmodels

Simple local models for quick compiler experiments.

## PyTorch: Matmul + Bias + ReLU + Elementwise chain

Script:
- `testmodels/simple_pytorch_matmul_bias_relu_elementwise.py`

Run:

```bash
python3 testmodels/simple_pytorch_matmul_bias_relu_elementwise.py \
  --batch 4 --in-features 16 --out-features 32 \
  --export testmodels/matmul_bias_relu_elementwise.ts
```

This script:
- Builds a small `nn.Module` with:
  - `matmul`
  - `+ bias`
  - `relu`
  - elementwise chain (`mul -> add -> sigmoid -> mul -> add`)
- Runs one inference with random input.
- Exports a TorchScript module.

## Compile with IREE

Prereqs:
- Python packages: `torch`, `iree-compiler`, `iree-turbine`
- Optional ONNX path: `onnx`, `onnxscript`, and `iree-import-onnx` in `PATH`

Command:

```bash
python3 testmodels/simple_pytorch_matmul_bias_relu_elementwise.py \
  --compile-iree \
  --iree-mode turbine \
  --iree-backend llvm-cpu \
  --iree-mlir testmodels/matmul_bias_relu_elementwise.mlir \
  --iree-vmfb testmodels/matmul_bias_relu_elementwise.vmfb
```

Outputs:
- TorchScript: `*.ts`
- MLIR: `*.mlir`
- IREE binary module: `*.vmfb` (compiled via `iree.compiler.compile_file` in Python)

ONNX fallback mode:

```bash
python3 testmodels/simple_pytorch_matmul_bias_relu_elementwise.py \
  --compile-iree \
  --iree-mode onnx \
  --iree-backend llvm-cpu \
  --iree-llvmcpu-target-cpu generic \
  --onnx-export testmodels/matmul_bias_relu_elementwise.onnx \
  --iree-mlir testmodels/matmul_bias_relu_elementwise.mlir \
  --iree-vmfb testmodels/matmul_bias_relu_elementwise.vmfb
```
