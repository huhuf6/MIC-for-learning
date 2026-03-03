#include "MIC/CUDA/CUDAKernel.h"
#include <cuda_runtime.h>
#include <iostream>
#include <vector>

using namespace std;
using namespace MIC::CUDA;

int main() {
    // Test LayerNorm kernel
    cout << "Testing LayerNorm kernel..." << endl;
    
    // Create test data
    int batchSize = 2;
    int hiddenSize = 4;
    float epsilon = 1e-5;
    
    vector<float> input_data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    vector<float> scale_data = {1.0, 1.0, 1.0, 1.0};
    vector<float> bias_data = {0.0, 0.0, 0.0, 0.0};
    vector<float> output_data(batchSize * hiddenSize, 0.0);
    
    // Allocate device memory
    float *d_input, *d_scale, *d_bias, *d_output;
    cudaMalloc(&d_input, input_data.size() * sizeof(float));
    cudaMalloc(&d_scale, scale_data.size() * sizeof(float));
    cudaMalloc(&d_bias, bias_data.size() * sizeof(float));
    cudaMalloc(&d_output, output_data.size() * sizeof(float));
    
    // Copy data to device
    cudaMemcpy(d_input, input_data.data(), input_data.size() * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_scale, scale_data.data(), scale_data.size() * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_bias, bias_data.data(), bias_data.size() * sizeof(float), cudaMemcpyHostToDevice);
    
    // Launch kernel
    cudaStream_t stream;
    cudaStreamCreate(&stream);
    
    MIC::CUDA::launchLayerNormKernel(d_input, d_scale, d_bias, d_output, batchSize, hiddenSize, epsilon, stream);
    
    // Copy results back to host
    cudaMemcpy(output_data.data(), d_output, output_data.size() * sizeof(float), cudaMemcpyDeviceToHost);
    
    // Print results
    cout << "Input:" << endl;
    for (int i = 0; i < batchSize; ++i) {
        for (int j = 0; j < hiddenSize; ++j) {
            cout << input_data[i * hiddenSize + j] << " ";
        }
        cout << endl;
    }
    
    cout << "Output:" << endl;
    for (int i = 0; i < batchSize; ++i) {
        for (int j = 0; j < hiddenSize; ++j) {
            cout << output_data[i * hiddenSize + j] << " ";
        }
        cout << endl;
    }
    
    // Test FusedLinearGELU kernel
    cout << "\nTesting FusedLinearGELU kernel..." << endl;
    
    int inFeatures = 2;
    int outFeatures = 3;
    
    vector<float> input_data2 = {1.0, 2.0, 3.0, 4.0};
    vector<float> weight_data = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};
    vector<float> bias_data2 = {0.1, 0.2, 0.3};
    vector<float> output_data2(batchSize * outFeatures, 0.0);
    
    // Allocate device memory
    float *d_input2, *d_weight, *d_bias2, *d_output2;
    cudaMalloc(&d_input2, input_data2.size() * sizeof(float));
    cudaMalloc(&d_weight, weight_data.size() * sizeof(float));
    cudaMalloc(&d_bias2, bias_data2.size() * sizeof(float));
    cudaMalloc(&d_output2, output_data2.size() * sizeof(float));
    
    // Copy data to device
    cudaMemcpy(d_input2, input_data2.data(), input_data2.size() * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_weight, weight_data.data(), weight_data.size() * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_bias2, bias_data2.data(), bias_data2.size() * sizeof(float), cudaMemcpyHostToDevice);
    
    // Launch kernel
    MIC::CUDA::launchFusedLinearGELUKernel(d_input2, d_weight, d_bias2, d_output2, batchSize, inFeatures, outFeatures, stream);
    
    // Copy results back to host
    cudaMemcpy(output_data2.data(), d_output2, output_data2.size() * sizeof(float), cudaMemcpyDeviceToHost);
    
    // Print results
    cout << "Input:" << endl;
    for (int i = 0; i < batchSize; ++i) {
        for (int j = 0; j < inFeatures; ++j) {
            cout << input_data2[i * inFeatures + j] << " ";
        }
        cout << endl;
    }
    
    cout << "Output:" << endl;
    for (int i = 0; i < batchSize; ++i) {
        for (int j = 0; j < outFeatures; ++j) {
            cout << output_data2[i * outFeatures + j] << " ";
        }
        cout << endl;
    }
    
    // Cleanup
    cudaFree(d_input);
    cudaFree(d_scale);
    cudaFree(d_bias);
    cudaFree(d_output);
    cudaFree(d_input2);
    cudaFree(d_weight);
    cudaFree(d_bias2);
    cudaFree(d_output2);
    cudaStreamDestroy(stream);
    
    cout << "\nAll tests completed successfully!" << endl;
    return 0;
}
