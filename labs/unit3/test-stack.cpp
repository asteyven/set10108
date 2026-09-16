#include <iostream>
#include <memory>
#include <thread>
#include "threadsafe_stack.h"

using namespace std;

void pusher(shared_ptr<threadsafe_stack<unsigned int>> stack)
{
    // Pusher will push 1 million values onto the stack
    for (unsigned int i = 0; i < 1000000; ++i)
    {
        stack->push(i);
        // Make the pusher yield.  Will give priority to another thread
        this_thread::yield();
    }
}

void popper(shared_ptr<threadsafe_stack<unsigned int>> stack)
{
    // Popper will pop 1 million values from the stack.
    // We do this using a counter and a while loop
    unsigned int count = 0;
    while (count < 1000000)
    {
        // Try and pop a value
        try
        {
            auto val = stack->pop();
            // Item popped.  Increment count
            ++count;
        }
        catch (exception e)
        {
            // Item not popped.  Display message
            cout << e.what() << endl;
        }
    }
}

int main(int argc, char **argv)
{
    // Create a threadsafe_stack
    auto stack = make_shared<threadsafe_stack<unsigned int>>();

    // Create two threads
    thread t1(popper, stack);
    thread t2(pusher, stack);

    // Join two threads
    t1.join();
    t2.join();

    // Check if stack is empty
    cout << "Stack empty = " << stack->empty() << endl;

    return 0;
}