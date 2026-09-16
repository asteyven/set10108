#include <thread>
#include <cstdio>
#include <climits>
#include <unistd.h>

int main(int argc, char ** argv)
{   
    const auto num_threads = std::thread::hardware_concurrency();
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    
    printf("Hello from %s, with %d threads\n", hostname, int(num_threads));
    return 0;
}