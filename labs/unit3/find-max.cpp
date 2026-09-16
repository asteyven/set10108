#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <future>

using namespace std;
using namespace std::chrono;

unsigned int find_max(const vector<unsigned int> &data, size_t start, size_t end)
{
    // Set max initially to 0
    unsigned int max = 0;
    // Iterate across vector from start to end position, setting max accordingly
    for (unsigned int i = start; i < end; ++i)
        if (data.at(i) > max)
            max = data.at(i);

    // Return max
    return max;
}

int main(int argc, char **argv)
{
    // Get the number of supported threads
    auto num_threads = thread::hardware_concurrency();

    // Create a vector with 2^24 random values
    vector<unsigned int> values;
    auto millis = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    default_random_engine e(static_cast<unsigned int>(millis.count()));
    for (unsigned int i = 0; i < pow(2, 24); ++i)
        values.push_back(e());

    // Create num threads - 1 futures
    vector<future<unsigned int>> futures;
    auto range = static_cast<size_t>(pow(2, 24) / num_threads);
    for (size_t i = 0; i < num_threads - 1; ++i)
        // Range is used to determine number of values to process
        futures.push_back(async(find_max, ref(values), i * range, (i + 1) * range));

    // Main application thread will process the end of the list
    auto max = find_max(values, (num_threads - 1) * range, num_threads * range);

    // Now get the results from the futures, setting max accordingly
    for (auto &f : futures)
    {
        auto result = f.get();
        if (result > max)
            max = result;
    }

    cout << "Maximum value found: " << max << endl;

    return 0;
}