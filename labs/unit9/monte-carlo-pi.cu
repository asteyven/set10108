#include <iostream>
#include <vector>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <curand.h>

using namespace std;

constexpr unsigned int ITERATIONS = 1 << 26;
constexpr unsigned int ITERATIONS_KERNEL = 1 << 16;
constexpr unsigned int TOTAL_KERNELS = ITERATIONS / ITERATIONS_KERNEL;

__global__ void monte_carlo_pi(const float2 *points, char *results)
{
	// Calculate index
	unsigned int idx = (blockIdx.x * blockDim.x) + threadIdx.x;

	// Get the point to work on
	float2 point = points[idx];
	// Calculate the length - not built-in
	float l = sqrtf((point.x * point.x) + (point.y * point.y));
	// Check if in circle
	if (l <= 1.0)
		results[idx] = 1;
	else
		results[idx] = 0;
}

__global__ void monte_carlo_pi(unsigned int iterations, float2 *points, int *results)
{
	// Calculate index
	unsigned int idx = (blockIdx.x * blockDim.x) + threadIdx.x;
	// Calculate starting point
	unsigned int start = idx * iterations;
	// Calculate end point
	unsigned int end = start + iterations;
	// Set starting result to 0
	results[idx] = 0;
	// Loop for iterations
	for (unsigned int i = start; i < end; ++i)
	{
		// Get the point to work on
		float2 point = points[i];
		// Calculate the length
		float l = sqrtf((point.x * point.x) + (point.y * point.y));
		// Check length and add to result accordingly
		if (l <= 1.0f)
			++results[idx];
	}
}

int main(int argc, char **argv)
{
	// Allocate host memory for results
	// For first approach
	// vector<char> results(ITERATIONS);
	// For second approach
	vector<int> results(TOTAL_KERNELS);

	// Allocate device memory for random data
	float2 *point_buffer;
	cudaMalloc((void**)&point_buffer, sizeof(float2) * ITERATIONS);

	// Allocate device memory for results data
	// For first approach
	// char *result_buffer;
	// cudaMalloc((void**)&result_buffer, sizeof(char) * ITERATIONS);
	// For second approach
	int *result_buffer;
	cudaMalloc((void**)&result_buffer, sizeof(int) * TOTAL_KERNELS);

	// Create random values on the GPU
	// Create generator
	curandGenerator_t rnd;
	curandCreateGenerator(&rnd, CURAND_RNG_QUASI_SOBOL32);
	curandSetQuasiRandomGeneratorDimensions(rnd, 2);
	curandSetGeneratorOrdering(rnd, CURAND_ORDERING_QUASI_DEFAULT);

	// Generate random numbers - point_buffer is an allocated device buffer
	curandGenerateUniform(rnd, (float*)point_buffer, 2 * ITERATIONS);

	// Destroy generator
	curandDestroyGenerator(rnd);

	// Execute kernel
	monte_carlo_pi<<<TOTAL_KERNELS / 128, 128>>>(ITERATIONS_KERNEL, point_buffer, result_buffer);
	
	// Wait for kernel to complete
	cudaDeviceSynchronize();
	// Read output buffer back to host
	cudaMemcpy(&results[0], result_buffer, sizeof(int) * TOTAL_KERNELS, cudaMemcpyDeviceToHost);

	// Sum
	int in_circle = 0;
	for (auto &v : results)
		in_circle += v;
	float pi = (4.0f * static_cast<float>(in_circle)) / static_cast<float>(ITERATIONS);

	cout << "pi = " << pi << endl;

	cudaFree(result_buffer);
	cudaFree(point_buffer);

	return 0;
}