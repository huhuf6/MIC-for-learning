import torch
import torch.nn as nn
import os
import subprocess

class TestModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.linear = nn.Linear(1024, 2048)
        self.gelu = nn.GELU()
        self.layernorm = nn.LayerNorm(2048)
    
    def forward(self, x):
        x = self.linear(x)
        x = self.gelu(x)
        x = self.layernorm(x)
        return x

def export_onnx_model():
    """Export test model to ONNX"""
    print("Exporting test model to ONNX...")
    
    model = TestModel()
    model.eval()
    
    # Create dummy input
    dummy_input = torch.randn(1, 1024)
    
    # Export to ONNX
    onnx_path = "test_model.onnx"
    torch.onnx.export(
        model,
        dummy_input,
        onnx_path,
        export_params=True,
        opset_version=11,
        do_constant_folding=True,
        input_names=['input'],
        output_names=['output'],
        dynamic_axes={'input': {0: 'batch_size'}, 'output': {0: 'batch_size'}}
    )
    
    print(f"ONNX model exported to: {onnx_path}")
    return onnx_path

def run_mlir2trt(onnx_path):
    """Run mlir2trt tool on ONNX model"""
    print(f"Running mlir2trt on: {onnx_path}")
    
    # Assuming mlir2trt is in PATH
    cmd = f"mlir2trt {onnx_path}"
    print(f"Executing: {cmd}")
    
    try:
        result = subprocess.run(cmd, shell=True, check=True, capture_output=True, text=True)
        print("mlir2trt output:")
        print(result.stdout)
        if result.stderr:
            print("mlir2trt stderr:")
            print(result.stderr)
        
        # Check if engine was created
        engine_path = onnx_path.replace('.onnx', '.engine')
        if os.path.exists(engine_path):
            print(f"TensorRT engine created successfully: {engine_path}")
            return engine_path
        else:
            print("Error: TensorRT engine was not created")
            return None
    except subprocess.CalledProcessError as e:
        print(f"Error running mlir2trt: {e}")
        print(f"Output: {e.stdout}")
        print(f"Error: {e.stderr}")
        return None

def run_benchmark(engine_path):
    """Run benchmark on TensorRT engine"""
    print(f"Running benchmark on: {engine_path}")
    
    # Run benchmark script
    cmd = f"python {os.path.dirname(os.path.abspath(__file__))}/../benchmark/benchmark.py"
    print(f"Executing: {cmd}")
    
    try:
        result = subprocess.run(cmd, shell=True, check=True, capture_output=True, text=True)
        print("Benchmark output:")
        print(result.stdout)
        if result.stderr:
            print("Benchmark stderr:")
            print(result.stderr)
    except subprocess.CalledProcessError as e:
        print(f"Error running benchmark: {e}")
        print(f"Output: {e.stdout}")
        print(f"Error: {e.stderr}")

def main():
    """Run complete test pipeline"""
    print("=== Starting MIC Test Pipeline ===")
    
    # Step 1: Export ONNX model
    onnx_path = export_onnx_model()
    
    # Step 2: Run mlir2trt
    engine_path = run_mlir2trt(onnx_path)
    
    # Step 3: Run benchmark
    if engine_path:
        run_benchmark(engine_path)
    
    print("=== Test Pipeline Complete ===")

if __name__ == "__main__":
    main()
