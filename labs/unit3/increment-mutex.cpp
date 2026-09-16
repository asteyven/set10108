#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;

mutex mut;

void increment(shared_ptr<int> value)
{
    // Loop 1 million times, incrementing value
    for (unsigned int i = 0; i < 1000000; ++i)
    {
        // Acquire the lock
        mut.lock();
        // Increment value
        *value = *value + 1;
        // Release the lock
        mut.unlock();
    }
}

int main(int argc, char **argv)
{
    // Create a shared int value
    auto value = make_shared<int>(0);

    // Create number of threads hardware natively supports
    auto num_threads = thread::hardware_concurrency();
    vector<thread> threads;
    for (unsigned int i = 0; i < num_threads; ++i)
        threads.push_back(thread(increment, value));

    // Join the threads
    for (auto &t : threads)
        t.join();

    // Display the value
    cout << "Value = " << *value << endl;
}