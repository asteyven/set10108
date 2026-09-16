#include <thread>
#include <chrono>
#include <iostream>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

using namespace std;
using namespace std::chrono;
using namespace std::this_thread;

void task_runner(const char * id, int sleepTimeInSeconds1, int sleepTimeInSeconds2)
{
    fmt::print("Task {} starting\n", id);
    fmt::print("Task {} sleeping for {} seconds\n", id, sleepTimeInSeconds1);
    sleep_for(seconds(sleepTimeInSeconds1));
    fmt::print("Task {} awake\n", id);
    fmt::print("Task {} sleeping again for {} seconds\n", id, sleepTimeInSeconds2);
    sleep_for(seconds(sleepTimeInSeconds2));
    fmt::print("Task {} awake again\n", id);
    fmt::print("Task {} ending\n", id);
}

void task_one() { task_runner("one", 3, 5); }
void task_two() { task_runner("two", 2, 10); }

int main(int argc, char **argv)
{
    fmt::print("Starting task one\n");
    thread t1(task_one);
    fmt::print("Starting task two\n");
    thread t2(task_two);
    fmt::print("Joining task one\n");
    t1.join();
    fmt::print("Task one joined\n");
    fmt::print("Joining task two\n");
    t2.join();
    fmt::print("Task two joined\n");
    fmt::print("Exiting\n");
    return 0;
}