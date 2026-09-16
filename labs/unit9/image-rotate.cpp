#define __CL_ENABLE_EXCEPTIONS

#include <iostream>
#include <fstream>
#include <CL/cl.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace std;
using namespace cl;

constexpr float PI = 3.14159265359;

int main(int argc, char **argv)
{
	int width, height, n;
	auto imgdata = (int *)stbi_load("pic.png", &width, &height, &n, 4); // ask for 4 component data, since that's what GPUs like best.
	vector<int> image_data(imgdata, imgdata + (width * height));
	stbi_image_free(imgdata);

	// Create rest of host data
	vector<int> result_data(image_data.size());

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

		// Create device buffers
		Buffer bufImage(context, CL_MEM_READ_ONLY, sizeof(int) * image_data.size());
		Buffer bufResults(context, CL_MEM_WRITE_ONLY, sizeof(int) * result_data.size());

		// Write host data to device
		queue.enqueueWriteBuffer(bufImage, CL_TRUE, 0, sizeof(int) * image_data.size(), &image_data[0]);

		// Read in kernel source
		ifstream file("image-rotate.cl");
		string code(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));

		// Create program
		Program::Sources source(1, make_pair(code.c_str(), code.length() + 1));
		Program program(context, source);

		// Build program for devices
		program.build(devices);

		// Create the kernel
		Kernel kernel(program, "image_rotate");

		// Set kernel arguments
		float cos_theta = cos(PI / 4.0f);
		float sin_theta = sin(PI / 4.0f);
		kernel.setArg(0, bufResults);
		kernel.setArg(1, bufImage);
		kernel.setArg(2, width);
		kernel.setArg(3, height);
		kernel.setArg(4, cos_theta);
		kernel.setArg(5, sin_theta);

		// Execute the kernel
		NDRange global(width, height);
		NDRange local(1, 1);
		queue.enqueueNDRangeKernel(kernel, NullRange, global, local);

		// Read the output buffer back to the host
		queue.enqueueReadBuffer(bufResults, CL_TRUE, 0, result_data.size() * sizeof(cl_int), &result_data[0]);

		// Write out image data
		stbi_write_png("output.png", width, height, 4, result_data.data(), width*4);

		cout << "Finished" << endl;
	}
	catch (Error error)
	{
		cout << error.what() << "(" << error.err() << ")" << endl;
	}

	return 0;
}