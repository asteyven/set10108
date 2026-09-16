#define __CL_ENABLE_EXCEPTIONS

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <CL/cl.hpp>

using namespace std;
using namespace cl;

constexpr int ITERATIONS = 1<<26; // 2^26 is likely maximum size before mem allocation problems in vector
constexpr int GROUP_SIZE = 1<<10; // 2^6 = 64, a good starting size
constexpr std::size_t RAND_SIZE = sizeof(cl_float2) * ITERATIONS;
constexpr std::size_t RESULTS_SIZE = sizeof(int) * (ITERATIONS / GROUP_SIZE);

int main(int argc, char **argv)
{
    // Initialise memory
	vector<cl_float2> randoms(ITERATIONS);
	random_device r;
	default_random_engine e(r());
	uniform_real_distribution<float> dist;
	for (std::size_t idx = 0; idx < ITERATIONS; ++idx)
	{
		randoms[idx].x = dist(e);
		randoms[idx].y = dist(e);
	}

	vector<int> results(ITERATIONS / GROUP_SIZE);

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

        // Create the buffers
        Buffer bufRandom(context, CL_MEM_READ_ONLY, RAND_SIZE);
        Buffer bufResults(context, CL_MEM_READ_ONLY, RESULTS_SIZE);

        // Copy data to the GPU
        queue.enqueueWriteBuffer(bufRandom, CL_TRUE, 0, RAND_SIZE, &randoms[0]);

        // Read in kernel source
        ifstream file("monte-carlo-pi.cl");
        string code(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));
        
        // Create program
        Program::Sources source(1, make_pair(code.c_str(), code.length() + 1));
        Program program(context, source);

        // Build program for devices
        program.build(devices);

        // Create the kernel
        Kernel kernel(program, "monte_carlo_pi");

        // Set kernel arguments
        kernel.setArg(0, bufRandom);
		// This creates a local buffer of size GROUP_SIZE * sizeof(char) with no buffer set.
		// Will allocate this amount of local memory for each group.
		kernel.setArg(1, GROUP_SIZE * sizeof(char), nullptr); 
        kernel.setArg(2, bufResults);

        // Execute kernel
        NDRange global(ITERATIONS);
        NDRange local(GROUP_SIZE);
        queue.enqueueNDRangeKernel(kernel, NullRange, global, local);

        // Copy result back.
        queue.enqueueReadBuffer(bufResults, CL_TRUE, 0, RESULTS_SIZE, &results[0]);

        // Sum final results
		int in_circle = 0;
		for (auto &i : results)
			in_circle += i;
		// Calculate pi
		float pi = (4.0f * static_cast<float>(in_circle)) / static_cast<float>(ITERATIONS);

        cout << "pi = " << pi << endl;
    }
    catch (Error error)
    {
        cout << error.what() << "(" << error.err() << ")" << endl;
    }
    return 0;
}