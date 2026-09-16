#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "guarded.h"

using namespace std;

constexpr unsigned int NUM_ITERATIONS = 1000000;
constexpr unsigned int NUM_THREADS = 4;

void task(shared_ptr<guarded> g)
{
    // Increment guarded object NUM_ITERATIONS times
    for (unsigned int i = 0; i < NUM_ITERATIONS; ++i)
        g->increment();
}

int main(int argc, char **argv)
{
    // Create guarded object
    auto g = make_shared<guarded>();

    // Create threads
    vector<thread> threads;
    for (unsigned int i = 0; i < NUM_THREADS; ++i)
        threads.push_back(thread(task, g));
    // Join threads
    for (auto &t : threads)
        t.join();

    // Display value stored in guarded object
    cout << "Value = " << g->get_value() << endl;

    return 0;
}