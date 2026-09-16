#define __CL_ENABLE_EXCEPTIONS

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <CL/cl.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace std;
using namespace cl;

constexpr cl_uint DIMENSION = 8192;
constexpr cl_uint ELEMENTS = DIMENSION * DIMENSION;

int main(int argc, char **argv)
{
	// Host data
	vector<cl_char> results(ELEMENTS);

	try
	{
		// Get the platforms
		vector<Platform> platforms;
		Platform::get(&platforms);

		// Assume only one platform.  Get GPU devices.
		vector<Device> devices;
		platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);

		// Just to test, print out device 0 name
		cout << devices[0].getInfo<CL_DEVICE_NAME>() << endl;

		// Create a context with these devices
		Context context(devices);

		// Create a command queue for device 0
		CommandQueue queue(context, devices[0]);

		// Create device buffer
		Buffer bufResults(context, CL_MEM_WRITE_ONLY, ELEMENTS * sizeof(char));

		// Read in kernel source
		ifstream file("mandelbrot.cl");
		string code(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));

		// Create program
		Program::Sources source(1, make_pair(code.c_str(), code.length() + 1));
		Program program(context, source);

		// Build program for devices
		program.build(devices);

		// Create the kernel
		Kernel kernel(program, "mandelbrot");

		// Set the kernel arguments
		kernel.setArg(0, bufResults);

		// Execute the kernel
		NDRange global(DIMENSION, DIMENSION);
		NDRange local(1, 1);
		queue.enqueueNDRangeKernel(kernel, NullRange, global, local);

		// Read the output buffer back to the host
		queue.enqueueReadBuffer(bufResults, CL_TRUE, 0, ELEMENTS * sizeof(char), &results[0]);

		stbi_write_png("mandelbrot.png", DIMENSION, DIMENSION,1, results.data(), DIMENSION * 1);

		cout << "Finished" << endl;
	}
	catch (Error error)
	{
		cout << error.what() << "(" << error.err() << ")" << endl;
	}
	return 0;
}