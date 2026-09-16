#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#include "gpuErrchk.h"

int main(int argc, char **argv)
{
    // Initialise CUDA - select device
    gpuErrchk(cudaSetDevice(0));
    
    return 0;
}