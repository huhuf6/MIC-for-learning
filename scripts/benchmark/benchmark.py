import time
import numpy as np
import torch
import torch.nn as nn
import tensorrt as trt
import pycuda.driver as cuda
import pycuda.autoinit

class BenchmarkRunner:
    def __init__(self):
        self.batch_sizes = [1, 4, 8, 16, 32]
        self.input_shape = (1024,)
        self.warmup_iterations = 10
        self.benchmark_iterations = 100
    
    def benchmark_pytorch(self):
        """Benchmark PyTorch model"""
        print("=== Benchmarking PyTorch ===")
        
        # Create model
        model = nn.Sequential(
            nn.Linear(1024, 2048),
            nn.GELU(),
            nn.LayerNorm(2048)
        ).cuda()
        model.eval()
        
        results = []
        for batch_size in self.batch_sizes:
            # Create input
            input_tensor = torch.randn(batch_size, *self.input_shape, device='cuda')
            
            # Warmup
            for _ in range(self.warmup_iterations):
                with torch.no_grad():
                    _ = model(input_tensor)
            
            # Benchmark
            start_time = time.time()
            for _ in range(self.benchmark_iterations):
                with torch.no_grad():
                    _ = model(input_tensor)
            end_time = time.time()
            
            latency = (end_time - start_time) / self.benchmark_iterations * 1000  # ms
            throughput = batch_size * self.benchmark_iterations / (end_time - start_time)  # samples/s
            
            results.append((batch_size, latency, throughput))
            print(f"Batch size: {batch_size}, Latency: {latency:.4f} ms, Throughput: {throughput:.2f} samples/s")
        
        return results
    
    def benchmark_tensorrt(self, engine_path):
        """Benchmark TensorRT engine"""
        print("=== Benchmarking TensorRT ===")
        
        # Load engine
        with open(engine_path, "rb") as f:
            engine_data = f.read()
        
        runtime = trt.Runtime(trt.Logger(trt.Logger.WARNING))
        engine = runtime.deserialize_cuda_engine(engine_data)
        context = engine.create_execution_context()
        
        results = []
        for batch_size in self.batch_sizes:
            # Allocate memory
            input_size = batch_size * np.prod(self.input_shape) * np.dtype(np.float32).itemsize
            output_size = batch_size * 2048 * np.dtype(np.float32).itemsize
            
            d_input = cuda.mem_alloc(input_size)
            d_output = cuda.mem_alloc(output_size)
            bindings = [int(d_input), int(d_output)]
            
            # Create input
            input_data = np.random.randn(batch_size, *self.input_shape).astype(np.float32)
            
            # Warmup
            for _ in range(self.warmup_iterations):
                cuda.memcpy_htod(d_input, input_data)
                context.execute_async_v2(bindings, cuda.Stream().handle)
            
            # Benchmark
            start_time = time.time()
            for _ in range(self.benchmark_iterations):
                cuda.memcpy_htod(d_input, input_data)
                context.execute_async_v2(bindings, cuda.Stream().handle)
            end_time = time.time()
            
            latency = (end_time - start_time) / self.benchmark_iterations * 1000  # ms
            throughput = batch_size * self.benchmark_iterations / (end_time - start_time)  # samples/s
            
            results.append((batch_size, latency, throughput))
            print(f"Batch size: {batch_size}, Latency: {latency:.4f} ms, Throughput: {throughput:.2f} samples/s")
            
            # Free memory
            d_input.free()
            d_output.free()
        
        return results
    
    def run_all_benchmarks(self, engine_path):
        """Run all benchmarks and generate comparison table"""
        print("Starting benchmark suite...")
        
        pytorch_results = self.benchmark_pytorch()
        tensorrt_results = self.benchmark_tensorrt(engine_path)
        
        # Generate comparison table
        print("\n=== Performance Comparison ===")
        print("Batch Size | PyTorch Latency (ms) | TensorRT Latency (ms) | Speedup")
        print("-" * 80)
        
        for (bs_p, lat_p, _), (bs_t, lat_t, _) in zip(pytorch_results, tensorrt_results):
            assert bs_p == bs_t
            speedup = lat_p / lat_t
            print(f"{bs_p:10} | {lat_p:20.4f} | {lat_t:21.4f} | {speedup:7.2f}x")
        
        return {
            "pytorch": pytorch_results,
            "tensorrt": tensorrt_results
        }

if __name__ == "__main__":
    runner = BenchmarkRunner()
    runner.run_all_benchmarks("model.engine")
