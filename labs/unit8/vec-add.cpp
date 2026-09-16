#define __CL_ENABLE_EXCEPTIONS

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <CL/cl.hpp>

using namespace std;
using namespace cl;

constexpr int ELEMENTS = 2048;
constexpr std::size_t DATA_SIZE = sizeof(int) * ELEMENTS;

int main(int argc, char **argv)
{
    // Initialise memory
    array<int, ELEMENTS> A;
    array<int, ELEMENTS> B;
    array<int, ELEMENTS> C;
    for (std::size_t i = 0; i < ELEMENTS; ++i)
        A[i] = B[i] = i;

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
        Buffer bufA(context, CL_MEM_READ_ONLY, DATA_SIZE);
        Buffer bufB(context, CL_MEM_READ_ONLY, DATA_SIZE);
        Buffer bufC(context, CL_MEM_WRITE_ONLY, DATA_SIZE);

        // Copy data to the GPU
        queue.enqueueWriteBuffer(bufA, CL_TRUE, 0, DATA_SIZE, &A);
        queue.enqueueWriteBuffer(bufB, CL_TRUE, 0, DATA_SIZE, &B);

        // Read in kernel source
        ifstream file("vec-add.cl");
        string code(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));
        
        // Create program
        Program::Sources source = { code };
        Program program(context, source);

        // Build program for devices
        program.build(devices);

        // Create the kernel
        Kernel vecadd_kernel(program, "vecadd");

        // Set kernel arguments
        vecadd_kernel.setArg(0, bufA);
        vecadd_kernel.setArg(1, bufB);
        vecadd_kernel.setArg(2, bufC);

        // Execute kernel
        NDRange global(ELEMENTS);
        NDRange local(256);
        queue.enqueueNDRangeKernel(vecadd_kernel, NullRange, global, local);

        // Copy result back.
        queue.enqueueReadBuffer(bufC, CL_TRUE, 0, DATA_SIZE, &C);

        // Test that the results are correct
        for (int i = 0; i < 2048; ++i)
            if (C[i] != i + i)
                cout << "Error: " << i << endl;

        cout << "Finished" << endl;
    }
    catch (Error error)
    {
        cout << error.what() << "(" << error.err() << ")" << endl;
    }
    return 0;
}
