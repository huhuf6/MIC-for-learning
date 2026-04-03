#!/usr/bin/env python3
"""Simple PyTorch model for pass coverage experiments.

Model structure:
  Matmul + Bias + ReLU + Elementwise chain
"""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import subprocess

import iree.compiler as ireec
from iree.turbine import aot as turbine_aot
import torch
import torch.nn as nn


class MatmulBiasReluElementwiseNet(nn.Module):
    def __init__(self, in_features: int, out_features: int):
        super().__init__()
        # Matmul parameters.
        self.weight = nn.Parameter(torch.randn(in_features, out_features) * 0.02)
        self.bias = nn.Parameter(torch.zeros(out_features))

        # Elementwise chain parameters.
        self.scale = nn.Parameter(torch.ones(out_features) * 1.5)
        self.shift = nn.Parameter(torch.ones(out_features) * 0.1)
        self.gate = nn.Parameter(torch.ones(out_features) * 0.5)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        # 1) Matmul + Bias
        y = torch.matmul(x, self.weight) + self.bias

        # 2) ReLU
        y = torch.relu(y)

        # 3) Elementwise chain: mul -> add -> sigmoid -> mul -> add
        z = y * self.scale
        z = z + self.shift
        z = torch.sigmoid(z) * z
        out = z + (y * self.gate)
        return out


def build_model(batch: int, in_features: int, out_features: int, seed: int):
    torch.manual_seed(seed)
    model = MatmulBiasReluElementwiseNet(in_features, out_features).eval()
    example_input = torch.randn(batch, in_features)
    return model, example_input


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--batch", type=int, default=4)
    parser.add_argument("--in-features", type=int, default=16)
    parser.add_argument("--out-features", type=int, default=32)
    parser.add_argument("--seed", type=int, default=0)
    parser.add_argument(
        "--export",
        type=Path,
        default=Path("testmodels/matmul_bias_relu_elementwise.ts"),
        help="Path to export TorchScript module",
    )
    parser.add_argument(
        "--compile-iree",
        action="store_true",
        help="Compile to IREE VMFB",
    )
    parser.add_argument(
        "--iree-mode",
        type=str,
        choices=["turbine", "onnx"],
        default="turbine",
        help="Compilation mode: direct turbine export or ONNX import path",
    )
    parser.add_argument(
        "--onnx-export",
        type=Path,
        default=Path("testmodels/matmul_bias_relu_elementwise.onnx"),
        help="Path to export ONNX model (used when --iree-mode=onnx)",
    )
    parser.add_argument(
        "--iree-mlir",
        type=Path,
        default=Path("testmodels/matmul_bias_relu_elementwise.mlir"),
        help="Path for MLIR dump (turbine export or ONNX-imported MLIR)",
    )
    parser.add_argument(
        "--iree-vmfb",
        type=Path,
        default=Path("testmodels/matmul_bias_relu_elementwise.vmfb"),
        help="Path for compiled IREE VMFB file (used with --compile-iree)",
    )
    parser.add_argument(
        "--iree-backend",
        type=str,
        default="llvm-cpu",
        help="IREE backend passed to --iree-hal-target-backends",
    )
    parser.add_argument(
        "--iree-llvmcpu-target-cpu",
        type=str,
        default="generic",
        help="Value for --iree-llvmcpu-target-cpu when backend is llvm-cpu",
    )
    args = parser.parse_args()

    model, x = build_model(
        batch=args.batch,
        in_features=args.in_features,
        out_features=args.out_features,
        seed=args.seed,
    )

    with torch.no_grad():
        y = model(x)

    print("input shape :", tuple(x.shape))
    print("output shape:", tuple(y.shape))
    print("output sample[0, :8]:", y[0, :8])

    args.export.parent.mkdir(parents=True, exist_ok=True)
    ts = torch.jit.trace(model, x)
    ts.save(str(args.export))
    print(f"saved TorchScript to: {args.export}")

    if not args.compile_iree:
        return

    args.iree_mlir.parent.mkdir(parents=True, exist_ok=True)
    args.iree_vmfb.parent.mkdir(parents=True, exist_ok=True)

    compile_extra_args = []
    if args.iree_backend == "llvm-cpu":
        compile_extra_args.append(
            f"--iree-llvmcpu-target-cpu={args.iree_llvmcpu_target_cpu}"
        )

    if args.iree_mode == "turbine":
        export_output = turbine_aot.export(model, x)
        export_output.save_mlir(args.iree_mlir)
        print(f"saved turbine MLIR to: {args.iree_mlir}")
        export_output.compile(
            save_to=args.iree_vmfb, target_backends=(args.iree_backend,)
        )
        print(f"saved IREE VMFB to: {args.iree_vmfb}")
        return

    # ONNX fallback path.
    if shutil.which("iree-import-onnx") is None:
        raise RuntimeError("iree-import-onnx not found in PATH")
    try:
        import onnx  # noqa: F401
    except Exception as exc:
        raise RuntimeError(
            "onnx Python package is required for --iree-mode=onnx. "
            "Install it with: pip install onnx onnxscript"
        ) from exc

    args.onnx_export.parent.mkdir(parents=True, exist_ok=True)
    torch.onnx.export(
        model,
        (x,),
        str(args.onnx_export),
        input_names=["input"],
        output_names=["output"],
        opset_version=18,
        do_constant_folding=True,
    )
    print(f"saved ONNX to: {args.onnx_export}")
    subprocess.run(
        [
            "iree-import-onnx",
            str(args.onnx_export),
            "-o",
            str(args.iree_mlir),
        ],
        check=True,
    )
    print(f"saved imported MLIR to: {args.iree_mlir}")
    vmfb = ireec.compile_file(
        str(args.iree_mlir),
        target_backends=[args.iree_backend],
        extra_args=compile_extra_args,
        output_format=ireec.OutputFormat.FLATBUFFER_BINARY,
    )
    args.iree_vmfb.write_bytes(vmfb)
    print(f"saved IREE VMFB to: {args.iree_vmfb}")


if __name__ == "__main__":
    main()
