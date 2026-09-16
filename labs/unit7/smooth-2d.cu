#include <iostream>
#include <vector>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace std;

// 5x5 gaussian kernel radius
constexpr int RADIUS = 2;

__global__ void smooth(float * values_in, float * values_out, int width, int height)
{
	// 5x5 Gaussian kernel. It's 1D and has 25 elements, although here it's made to look like 2D because of the newlines
	constexpr float KERNEL[] = {
		0.003765, 	0.015019, 	0.023792, 	0.015019, 	0.003765,
		0.015019, 	0.059912, 	0.094907, 	0.059912, 	0.015019,
		0.023792, 	0.094907, 	0.150342, 	0.094907, 	0.023792,
		0.015019, 	0.059912, 	0.094907, 	0.059912, 	0.015019,
		0.003765, 	0.015019, 	0.023792, 	0.015019, 	0.003765,
	};

	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;

	// prevent out-of-bounds
	if (x >= width || y >= height)
		return;

	// temporary variable to put the weighted sum
	float tmp = 0.0f;
	for (int oy = -RADIUS; oy <= RADIUS; ++oy)
	{
		// clamp the value to the boundary value. Instead of going out of bounds, we're going to be using the last element possible.
		int ynb = min(max(y + oy,0), height-1);
		int kernel_row = oy + RADIUS; // in the range of [0,2*RADIUS]
		for (int ox = -RADIUS; ox <= RADIUS; ++ox)
		{
			int xnb = min(max(x + ox,0),width-1);
			int kernel_col = ox + RADIUS;// in the range of [0,2*RADIUS]
			float value = values_in[xnb + ynb * width] * KERNEL[kernel_col + kernel_row*5];
			tmp += value;
		}
	}
	values_out[x + y * width] = tmp;
}

std::vector<float> load_image_to_grayscale(const char* filename, int& width, int& height)
{
	int n;
	auto imgdata = (uint8_t*)stbi_load(filename, &width, &height, &n, 0); 
	vector<float> values(width*height);
	for(int y=0;y<height; ++y)
		for (int x = 0; x < width; ++x)
		{
			int o = x + y * width; // 1d index
			// is it RGB or RGBA?
			if (n >= 3)
			{
				float r = imgdata[o * n] / 255.0f;
				float g = imgdata[o * n+1] / 255.0f;
				float b = imgdata[o * n+2] / 255.0f;
				values[o] = 0.2126f * r + 0.7152f * g + 0.0722f * b; // RGB -> grayscale conversion formula
			}
			else if (n == 1)
			{
				values[o] = imgdata[o]/255.0f;
			}
		}
	stbi_image_free(imgdata);
	return values;
}

void save_grayscale_png(const char* filename, const std::vector<float>& values, int width, int height)
{
	std::vector<uint8_t> imgdata(width * height);
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x)
			imgdata[x + y * width] = uint8_t(values[x + y * width] * 255);
	stbi_write_png(filename, width, height, 1, imgdata.data(), width);
}

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		printf("Please provide an image filename as the argument (full path). The program will create a new image with the suffix _out.png and will place it alongside the original image");
		exit(0);
	}
	int width, int height;
	std::string filename_in = argv[1];
	auto h_values_in = load_image_to_grayscale(filename_in.c_str(), width, height);
	const size_t NUM_PIXELS = h_values_in.size();

	// Create host memory
	const size_t NUM_BYTES = sizeof(float) * NUM_PIXELS;
	vector<float> h_values_out(NUM_PIXELS);    // Output array

	float* d_values_in = nullptr;
	float* d_values_out = nullptr;

	// Initialise buffers
	cudaMalloc((void**)&d_values_in, NUM_BYTES);
	cudaMalloc((void**)&d_values_out, NUM_BYTES);

	// Write host data to device
	cudaMemcpy(d_values_in, h_values_in.data(), NUM_BYTES, cudaMemcpyHostToDevice);
	cudaMemcpy(d_values_out, h_values_out.data(), NUM_BYTES, cudaMemcpyHostToDevice);

	// arbitrary! Change it to something else. Is it better? Worse?
	dim3 blockDim = { 32,32,1 };
	// This is written to accomodate the blockDim we specified and the width/height of the image.
	// If unsure why the +31, check the lecture. We need to make sure we have enough threads, but not blockDim or more
	dim3 gridDim = { 
		(unsigned(width) + 31) / 32, 
		(unsigned(height) + 31) / 32, 
		1 
	};
	smooth <<<gridDim, blockDim >>>(d_values_in, d_values_out, width, height);

	// Read output buffer back to the host
	cudaMemcpy(h_values_out.data(), d_values_out, NUM_BYTES, cudaMemcpyDeviceToHost);

	// Clean up resources
	cudaFree(d_values_in);
	cudaFree(d_values_out);

	auto filename_out = filename_in.substr(0, filename_in.size() - 4) + "_out.png";
	save_grayscale_png(filename_out.c_str(), h_values_out, width, height);

	return 0;
}
