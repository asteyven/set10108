#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>

using namespace std;
using namespace std::chrono;

void task(unsigned int id, atomic_flag& flag)
{
    // Do 10 iterations
    for (unsigned int i = 0; i < 10; ++i)
    {
        // Test the flag is available, and grab when it is
        // Notice this while loops keeps spinning until flag is clear
        while (flag.test_and_set());
        // Flag is available.  Thread displays message
        cout << "Thread " << id << " running " << i << endl;
        // Sleep for 1 second
        this_thread::sleep_for(seconds(1));
        // Clear the flag
        flag.clear();
    }
}

int main(int argc, char **argv)
{
    // Create shared flag
    atomic_flag flag;
    flag.clear();

    // Get number of hardware threads
    auto num_threads = thread::hardware_concurrency();

    // Create threads
    vector<thread> threads;
    for (unsigned int i = 0; i < num_threads; ++i)
        threads.push_back(thread(task, i, ref(flag)));

    // Join threads
    for (auto &t : threads)
        t.join();

    return 0;
}