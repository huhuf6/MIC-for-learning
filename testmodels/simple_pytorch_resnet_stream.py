#!/usr/bin/env python3
"""Export a small ResNet-style model and compile it up to Stream IR.

This avoids a torchvision dependency by defining a minimal ResNet18-like model
locally. The goal is compiler inspection, not pretrained accuracy.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import subprocess

from iree.turbine import aot as turbine_aot
import torch
import torch.nn as nn


class BasicBlock(nn.Module):
    expansion = 1

    def __init__(self, in_channels: int, out_channels: int, stride: int = 1):
        super().__init__()
        self.conv1 = nn.Conv2d(
            in_channels,
            out_channels,
            kernel_size=3,
            stride=stride,
            padding=1,
            bias=False,
        )
        self.bn1 = nn.BatchNorm2d(out_channels)
        self.relu = nn.ReLU(inplace=False)
        self.conv2 = nn.Conv2d(
            out_channels,
            out_channels,
            kernel_size=3,
            stride=1,
            padding=1,
            bias=False,
        )
        self.bn2 = nn.BatchNorm2d(out_channels)
        if stride != 1 or in_channels != out_channels:
            self.downsample = nn.Sequential(
                nn.Conv2d(
                    in_channels,
                    out_channels,
                    kernel_size=1,
                    stride=stride,
                    bias=False,
                ),
                nn.BatchNorm2d(out_channels),
            )
        else:
            self.downsample = nn.Identity()

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        identity = self.downsample(x)
        out = self.conv1(x)
        out = self.bn1(out)
        out = self.relu(out)
        out = self.conv2(out)
        out = self.bn2(out)
        out = out + identity
        out = self.relu(out)
        return out


class SmallResNet(nn.Module):
    def __init__(self, num_classes: int = 1000):
        super().__init__()
        self.stem = nn.Sequential(
            nn.Conv2d(3, 64, kernel_size=7, stride=2, padding=3, bias=False),
            nn.BatchNorm2d(64),
            nn.ReLU(inplace=False),
            nn.MaxPool2d(kernel_size=3, stride=2, padding=1),
        )
        self.layer1 = self._make_layer(64, 64, blocks=2, stride=1)
        self.layer2 = self._make_layer(64, 128, blocks=2, stride=2)
        self.layer3 = self._make_layer(128, 256, blocks=2, stride=2)
        self.layer4 = self._make_layer(256, 512, blocks=2, stride=2)
        self.avgpool = nn.AdaptiveAvgPool2d((1, 1))
        self.fc = nn.Linear(512, num_classes)

    def _make_layer(
        self, in_channels: int, out_channels: int, blocks: int, stride: int
    ) -> nn.Sequential:
        layers = [BasicBlock(in_channels, out_channels, stride=stride)]
        for _ in range(1, blocks):
            layers.append(BasicBlock(out_channels, out_channels, stride=1))
        return nn.Sequential(*layers)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        x = self.stem(x)
        x = self.layer1(x)
        x = self.layer2(x)
        x = self.layer3(x)
        x = self.layer4(x)
        x = self.avgpool(x)
        x = torch.flatten(x, 1)
        x = self.fc(x)
        return x


def build_model(seed: int, num_classes: int) -> tuple[nn.Module, torch.Tensor]:
    torch.manual_seed(seed)
    model = SmallResNet(num_classes=num_classes).eval()
    example_input = torch.randn(1, 3, 224, 224)
    return model, example_input


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--seed", type=int, default=0)
    parser.add_argument("--num-classes", type=int, default=1000)
    parser.add_argument(
        "--torch-mlir",
        type=Path,
        default=Path("testmodels/resnet18_like_turbine.mlir"),
        help="Path to save the turbine-exported MLIR",
    )
    parser.add_argument(
        "--stream-mlir",
        type=Path,
        default=Path("testmodels/resnet18_like_before_hal_stream.mlir"),
        help="Path to save the IR compiled up to the stream phase",
    )
    parser.add_argument(
        "--compile-to",
        type=str,
        default="stream",
        choices=["flow", "stream", "hal"],
        help="Phase to stop compilation at",
    )
    args = parser.parse_args()

    model, example_input = build_model(args.seed, args.num_classes)
    with torch.no_grad():
        output = model(example_input)
    print("input shape :", tuple(example_input.shape))
    print("output shape:", tuple(output.shape))
    print("output sample[0, :8]:", output[0, :8])

    args.torch_mlir.parent.mkdir(parents=True, exist_ok=True)
    args.stream_mlir.parent.mkdir(parents=True, exist_ok=True)

    export_output = turbine_aot.export(model, example_input)
    export_output.save_mlir(args.torch_mlir)
    print(f"saved turbine MLIR to: {args.torch_mlir}")

    subprocess.run(
        [
            "iree-compile",
            str(args.torch_mlir),
            f"--compile-to={args.compile_to}",
            "-o",
            str(args.stream_mlir),
        ],
        check=True,
    )
    print(f"saved {args.compile_to} IR to: {args.stream_mlir}")


if __name__ == "__main__":
    main()
