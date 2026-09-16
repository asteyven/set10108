#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>
#include <future>
#include <thread>
#include <fstream>

using namespace std;
using namespace std::chrono;

void monte_carlo_pi(size_t iterations, promise<double> pi)
{
    // Seed with real random number if available
    random_device r;
    // Create random number generator
    default_random_engine e(r());
    // Create a distribution - we want doubles between 0.0 and 1.0
    uniform_real_distribution<double> distribution(0.0, 1.0);

    // Keep track of number of points in circle
    unsigned int in_circle = 0;
    // Iterate
    for (size_t i = 0; i < iterations; ++i)
    {
        // Generate random point
        auto x = distribution(e);
        auto y = distribution(e);
        // Get length of vector defined - use Pythagarous
        auto length = sqrt((x * x) + (y * y));
        // Check if in circle
        if (length <= 1.0)
            ++in_circle;
    }
    // Calculate pi
    pi.set_value((4.0 * in_circle) / static_cast<double>(iterations));
}

int main(int argc, char **argv)
{
    // Create data file
    ofstream data("montecarlo.csv", ofstream::out);

    for (size_t num_threads = 0; num_threads <= 6; ++num_threads)
    {
        auto total_threads = static_cast<unsigned int>(pow(2.0, num_threads));
        // Write number of threads
        cout << "Number of threads = " << total_threads << endl;
        // Write number of threads to the file
        data << "num_threads_" << total_threads;
        // Now execute 100 iterations
        for (size_t iters = 0; iters < 100; ++iters)
        {
            // Get the start time
            auto start = system_clock::now();
            // We need to create total_threads threads
            vector<future<double>> futures;
            vector<thread> threads;
            for (size_t n = 0; n < total_threads; ++n)
            {
                // Create promise
                promise<double> pi;
                // Store future
                futures.push_back(pi.get_future());
                // Working in base 2 to make things a bit easier
                threads.push_back(thread(monte_carlo_pi, static_cast<unsigned int>(pow(2.0, 24.0 - num_threads)), move(pi)));
            }
            // Calculate pi from futures
            double pi = 0.0;
            for (auto &f : futures)
                pi += f.get();
            pi /= static_cast<double>(total_threads);
            // Join the threads (wait for them to finish)
            for (auto &t : threads)
                t.join();
            // Get the end time
            auto end = system_clock::now();
            // Get the total time
            auto total = end - start;
            // Print result
            cout << "pi = " << pi << " in " << duration_cast<milliseconds>(total).count() << " ms" << endl;
            // Convert to milliseconds and output to file
            data << ", " << duration_cast<milliseconds>(total).count();
        }
        data << endl;
    }
    // Close the file
    data.close();
    return 0;
}