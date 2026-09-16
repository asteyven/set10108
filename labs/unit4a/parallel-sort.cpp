#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <cmath>
#include <thread>

using namespace std;
using namespace std::chrono;

vector<unsigned int> generate_values(size_t size)
{
    // Create random engine
    random_device r;
    default_random_engine e(r());
    // Generate random numbers
    vector<unsigned int> data;
    for (size_t i = 0; i < size; ++i)
        data.push_back(e());
    return data;
}

void parallel_sort(vector<unsigned int>& values)
{
    // Get the number of threads
    auto num_threads = thread::hardware_concurrency();
    // Get the number of elements in the vector
    auto n = values.size();
    // Declare the variables used in the loop
    int i, phase;
    // Declare parallel section
#pragma omp parallel num_threads(num_threads) default(none) shared(values, n) private(i, phase)
    for (phase = 0; phase < n; ++phase)
    {
        // Determine which phase of the sort we are in
        if (phase % 2 == 0)
        {
            // Parallel for loop.  Each thread jumps forward 2 so no conflict
#pragma omp for
            for (i = 1; i < n; i += 2)
            {
                // Check if we should swap values
                if (values[i - 1] > values[i])
                    std::swap(values[i - 1], values[i]); // and swap
            }
        }
        else
        {
            // Parallel for loop.  Each thread jumps forward 2 so no conflict
#pragma omp for
            for (i = 1; i < (n-1); i += 2)
            {
                // Check is we should swap values
                if (values[i] > values[i + 1])
                    std::swap(values[i + 1], values[i]); // and swap
            }
        }
    }
}

int main(int argc, char **argv)
{
    // Create results file
    ofstream results("parallel.csv", ofstream::out);
    // Gather results for 2^6 to 2^12 results
    for (size_t size = 6; size <= 12; ++size)
    {
        // Output data size
        results << pow(2, size) << ", ";
        // Gather 100 results
        for (size_t i = 0; i < 100; ++i)
        {
            // Generate vector of random values
            cout << "Generating " << i << " for " << pow(2, size) << " values" << endl;
            auto data = generate_values(static_cast<size_t>(pow(2, size)));
            // Sort the vector
            cout << "Sorting" << endl;
            auto start = system_clock::now();
            parallel_sort(data);
            auto end = system_clock::now();
            auto total = duration_cast<milliseconds>(end - start).count();
            // Output time
            results << total << ",";
        }
        results << endl;
    }
    results.close();

    return 0;
}