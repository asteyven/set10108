#include <thread>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

using namespace std;
using namespace std::chrono;

constexpr size_t num_threads = 100;

void task(size_t n , int val)
{
    cout << "Thread: " << n << " Random Value: " << val << endl;
}

int main(int argc, char **argv)
{
    // C++ style of creating a random
    // Seed with real random number if available
    std::random_device r;
    // Create random number generator
    default_random_engine e(r());

    // Create 100 threads in a vector
    vector<thread> threads;
    for (size_t i = 0; i < num_threads; ++i)
        threads.push_back(thread(task, i, e()));

    // Use C++ ranged for loop to join the threads
    // Same as foreach in C#
    for (auto &t : threads)
        t.join();

    return 0;
}