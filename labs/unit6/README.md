# GPU Programming with CUDA

OpenCL is the equivalent of OpenGL for GPU programming. It runs on most hardware and has tools provided by a number of vendors.  However, generalisation comes at the cost of simplicity and performance.  CUDA can be considered the DirectX of GPU programming.  It is proprietary as it is provided by Nvidia to run on Nvidia hardware.  There is a simplicity and performance benefit because of the proprietary nature of CUDA. You will find the general ideas are the same as OpenCL just fewer lines of code to get going.

## Getting Started with CUDA

If you have the CUDA SDK installed, you should have Nvidia Nsight added to Visual Studio.  This being the case, you will be able to create a new CUDA application by selecting it during the project creation (under NVIDIA in the templates). Visual Studio will provide an initial kernel.  You should delete this code and start from an empty file. Otherwise, you can copy this CMake project and use it as a starting point.

First we we need to initialise CUDA.

```cuda
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#include "gpuErrchk.h"

int main(int argc, char **argv)
{
    // Initialise CUDA - select device
    gpuErrchk(cudaSetDevice(0));
    
    return 0;
}
```

The above uses a helper function, defined as follows:
```cpp
#pragma once
#include <cstdio>
#include <cstdlib>
#include <cuda_runtime.h>

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char* file, int line, bool abort = true)
{
	if (code != cudaSuccess)
	{
		fprintf(stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
		if (abort)
			exit(code);
	}
}
```

This will exit the application at the first CUDA error, which might be harsh, but could save you of confusion from errors adding up and causing undefined behaviours.

If you are using Linux, then the compile command is `nvcc` which calls `gcc` as needed.  Therefore the command line arguments you can use are similar.

And that is it. Much simpler than OpenCL as we only need to select a device. If your machine only has one Nvidia device, this is just 0. Run the application to test it just to ensure everything is set up OK. Let us now output some information from CUDA.

## Getting CUDA Info

Getting information from CUDA is also fairly trivial.

```cuda
#include <iostream>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#include "gpuErrchk.h"

using namespace std;


int main(int argc, char **argv)
{
    // Get number of devices on system
    int deviceCount; 
    gpuErrchk(cudaGetDeviceCount(&deviceCount));

    cout << "Number of devices: " << deviceCount << endl;
    for (int i = 0; i < deviceCount; ++i) 
    {
        // Get properties for device
        cudaDeviceProp deviceProp;
        cudaGetDeviceProperties(&deviceProp, i);

        cout << "Device " << i << endl;
        cout << "Name " << deviceProp.name << endl;
        cout << "Revision " << deviceProp.major << "." << deviceProp.minor << endl;
        cout << "Memory " << deviceProp.totalGlobalMem / 1024 / 1024 << "MB" << endl;
        cout << "Warp Size " << deviceProp.warpSize << endl;
        cout << "Clock " << deviceProp.clockRate << endl;
        cout << "Multiprocessors " << deviceProp.multiProcessorCount << endl;
    } 
    return 0;
}
```

An example output is:

```shell
Number of devices: 1
Device 0
Name GeForce GTX TITAN X
Revision 5.2
Memory 12288MB
Warp Size 32
Clock 1050MHz
Multiprocessors 24
```

This is the same type of information achieved from interrogating OpenCL. So why do Nvidia state that they have 3072 CUDA cores for the graphics card shown? Each multiprocessor (we have 24) has a number of streaming processors (processors that can execute an instruction for us). Each multiprocessor in the graphics card has 128 of these. Depending on the CUDA capability of your card, you will have a different number of streaming processors per multiprocessor. The table illustrates the various capabilities.

| **Microarchitecture** | **CUDA Capability** | **SP per MP** |
|-----------------------|---------------------|---------------|
| Tesla                 |  1.0 - 1.3          |          8    |
| Fermi                 |  2.0                |         32    |
| Fermi                 |  2.1                |         48    |
| Kepler                |  3.0 - 3.7          |        192    |
| Maxwell               |  5.0 - 5.3          |        128    |
| Pascal                |  6.0 - 6.2          |        128    |
| Volta                 |  7.0 - 7.2          |         64    |
| Turing                |  7.5                |               |
| Ampere                |  8.0                |               |

A GTX Titan X has Maxwell architecture and has 24 multiprocessors. This means that the Titan X will have 3072 cores in total, running at approximately 1050MHz. This means that we over 3.2 THz of performance (if that was how we measured such things). Turing introduces a new processor architecture, the Streaming Multiprocessor (SM).

## CUDA Kernels

One of the main differences CUDA provides from OpenCL is that we have a single file solution. That means we write our CUDA kernel in the same file as our main method. This keeps us closer to standard C++ development, and we do not need to load and compile external.

```cuda
__global__ void vecadd(const int *A, const int *B, int *C)
{
    // Get block index
    unsigned int block_idx = blockIdx.x;
    // Get thread index
    unsigned int thread_idx = threadIdx.x;
    // Get the number of threads per block
    unsigned int block_dim = blockDim.x;
    // Get the thread's unique ID - (block_idx * block_dim) + thread_idx;
    unsigned int idx = (block_idx * block_dim) + thread_idx;
    // Add corresponding locations of A and B and store in C
    C[idx] = A[idx] + B[idx];
}
```

## Passing Data to CUDA Kernels

We have to work between host memory (main memory) and device memory (GPU memory).  We first declare and initialise host memory.

```cuda
// Create host memory
auto data_size = sizeof(int) * ELEMENTS;
vector<int> A(ELEMENTS);    // Input aray
vector<int> B(ELEMENTS);    // Input array
vector<int> C(ELEMENTS);    // Output array

// Initialise input data
for (unsigned int i = 0; i < ELEMENTS; ++i)
    A[i] = B[i] = i;
```

To initialise memory on the device:

```cuda
// Declare buffers
int *buffer_A, *buffer_B, *buffer_C;

// Initialise buffers
cudaMalloc((void**)&buffer_A, data_size);
cudaMalloc((void**)&buffer_B, data_size);
cudaMalloc((void**)&buffer_C, data_size);
```

Notice we don't need special types with CUDA.  We declare an `int` pointer as standard. The difference is in how we allocate memory.  You may be familiar with `malloc` from standard C. `cudaMalloc` undertakes the same functionality but for allocating memory on the GPU.

To copy the memory from the host to the device we use `cudaMemcpy`:

```cuda
cudaMemcpy(dest, src, size, direction);
```

`direction` tell sCUDA which way the data is being copied (host to device, device to host, device to device).

```cuda
// Write host data to device
cudaMemcpy(buffer_A, &A[0], data_size, cudaMemcpyHostToDevice);
cudaMemcpy(buffer_B, &B[0], data_size, cudaMemcpyHostToDevice);
```

## Running and Getting Results from CUDA

To run a kernel:

```cuda
// Run kernel with one thread for each element
// First value is number of blocks, second is threads per block.  Max 1024 threads per block
vecadd<<<ELEMENTS / 1024, 1024>>>(buffer_A, buffer_B, buffer_C);

// Wait for kernel to complete
cudaDeviceSynchronize();
```

We define the number of blocks (`ELEMENTS / 1024`) and the number of threads per block (`1024`). The call to `cudaDeviceSynchronize` means we wait for the kernel to complete before continuing.

To get our results back we call `cudaMemcpy` again.

```cuda
// Read output buffer back to the host
cudaMemcpy(&C[0], buffer_C, data_size, cudaMemcpyDeviceToHost);
```

You should write the code to check that the output is correct. It is the same as the code in the OpenCL example.

## Freeing Resources

The last thing our application has to do is free any resources used by CUDA. As we have only allocated memory, this requires only a few calls.

```cuda
// Clean up resources
cudaFree(buffer_A);
cudaFree(buffer_B);
cudaFree(buffer_C);
```

Running this application should give you the same output as that in the OpenCL version.

## Matrix Multiplication

Your task here is to write and test the application required to multiply two matrices using CUDA.

```cuda
__global__ void simple_multiply(float *output_C, unsigned int width_A, unsigned int height_A, unsigned int width_B, unsigned int height_B, const float *input_A, const float *input_B)
{
    // Get global position in Y direction
    unsigned int row = (blockIdx.y * 1024) + threadIdx.y;
    // Get global position in X direction
    unsigned int col = (blockIdx.x * 1024) + threadIdx.x;
    
    float sum = 0.0f;
    
    // Calculate result of one element of matrix C
    for (unsigned int i = 0; i < width_A; ++i)
    sum += input_A[row * width_A + i] * input_B[i * width_B + col];
    
    // Store result in matrix C
    output_C[row * width_B + col] = sum;
}
```
