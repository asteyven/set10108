#include <iostream>
#include <numeric>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>
#include <thread>
#include <fstream>

using namespace std;
using namespace std::chrono;

void monte_carlo_pi(size_t iterations, size_t& in_circle)
{
    // Seed with real random number if available
    random_device r;
    // Create random number generator
    default_random_engine e(r());
    // Create a distribution - we want doubles between 0.0 and 1.0
    uniform_real_distribution<double> distribution(0.0, 1.0);

    // Keep track of number of points in circle
    in_circle = 0;
    // Iterate
    for (size_t i = 0; i < iterations; ++i)
    {
        // Generate random point
        auto x = distribution(e);
        auto y = distribution(e);
        // Get length of vector defined - use Pythagorean theorem
        auto length = sqrt((x * x) + (y * y));
        // Check if in circle
        if (length <= 1.0)
            ++in_circle;
    }
    
}

int main(int argc, char **argv)
{
    // Create data file
    ofstream data("montecarlo.csv", ofstream::out);
    data << "Number of threads,Time (ms)" << endl;
    for (size_t num_threads_power_of_two = 0; num_threads_power_of_two <= 10; ++num_threads_power_of_two)
    {
        auto total_threads = 1 << num_threads_power_of_two; // 2 ^ num_threads_power_of_two
        // Write number of threads
        cout << "Number of threads = " << total_threads << endl;
        // Write number of threads to the file
        data << total_threads;
        // Now execute some iterations and average results
        uint64_t total_ms = 0;
        static const size_t MAX_ITERS = 10;
        for (size_t iters = 0; iters < MAX_ITERS; ++iters)
        {
            // Get the start time
            auto start = system_clock::now();
            // We need to create total_threads threads
            vector<thread> threads;
            vector<size_t> in_cicle(total_threads);
            threads.reserve(total_threads);
            for (size_t n = 0; n < total_threads; ++n)
                // Working in base 2 to make things a bit easier
                threads.push_back(thread(monte_carlo_pi, 1 << (24 - num_threads_power_of_two), ref(in_cicle[n]) ));
            // Join the threads (wait for them to finish)
            for (auto &t : threads)
                t.join();
            // Calculate pi
            auto in_circle_total = std::accumulate(in_cicle.begin(), in_cicle.end(), size_t(0));
            auto pi = (4.0 * in_circle_total) / static_cast<double>(1 << 24);
            printf("PI: %f\n", float(pi));
            // Get the end time
            auto end = system_clock::now();
            // Get the total time
            auto total = end - start;
            // Convert to milliseconds and save
            total_ms += duration_cast<milliseconds>(total).count();
        }
        // output to file
        data << "," << total_ms / MAX_ITERS << endl;
    }
    // Close the file
    data.close();
    return 0;
}