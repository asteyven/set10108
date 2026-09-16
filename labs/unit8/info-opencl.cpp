#define __CL_ENABLE_EXCEPTIONS

#include <iostream>
#include <vector>
#include <CL/cl.hpp>

using namespace std;
using namespace cl;

int main(int argc, char **argv)
{
    try
    {
        // Get the platforms
        vector<Platform> platforms;
        Platform::get(&platforms);

        // Iterate through each platform
        for (auto &p : platforms)
        {
            // Print out platform name
            cout << "********************" << endl;
            cout << p.getInfo<CL_PLATFORM_NAME>() << endl;

            // Get all devices for the platform
            vector<Device> devices;
            p.getDevices(CL_DEVICE_TYPE_ALL, &devices);

            // Iterate through all the devices
            for (auto &d : devices)
            {
                cout << endl;
                cout << d.getInfo<CL_DEVICE_NAME>() << endl;
                cout << "Vendor: " << d.getInfo<CL_DEVICE_VENDOR>() << endl;
                cout << "Max Compute: " << d.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << endl;
                cout << "Max Memory: " << d.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / (1024 * 1024) << "MB" << endl;
                cout << "Clock Freq: " << d.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << "MHz" << endl;
                cout << "Available: " << (d.getInfo<CL_DEVICE_AVAILABLE>() ? "True" : "False") << endl;
                cout << "Max Item Size X " << d.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0]  << endl;
                cout << "Max Item Size Y " << d.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[1] << endl;
                cout << "Max Item Size Z " << d.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[2] << endl;
                cout << "Max Group Size " << d.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << endl;
            }
            
            cout << endl;
            cout << "********************" << endl;
        }
    }
    catch (Error error)
    {
        cout << error.what() << "(" << error.err() << ")" << endl;
    }
    return 0;
}
