#pragma once

#include <exception>
#include <stack>
#include <memory>
#include <mutex>

template<typename T>
class threadsafe_stack
{
private:
    // The actual stack object we are using
    std::stack<T> data;
    // The mutex to control access
    mutable std::mutex mut;
public:
    // Normal constructor
    threadsafe_stack() { }
    // Copy constructor
    threadsafe_stack(const threadsafe_stack &other)
    {
        // We need to copy the data from the other stack.  Lock other stack
        std::lock_guard<std::mutex> lock(other.mut);
        data = other.data;
    }
    // Push method.  Adds to the stack
    void push(T value)
    {
        // Lock access to the object
        std::lock_guard<std::mutex> lock(mut);
        // Push value onto the internal stack
        data.push(value);
    }
    // Pop method.  Removes from the stack
    T pop()
    {
        // Lock access to the object
        std::lock_guard<std::mutex> lock(mut);
        // Check if stack is empty
        if (data.empty()) throw std::exception();
        // Access value at the top of the stack.
        auto res = data.top();
        // Remove the top item from the stack
        data.pop();
        // Return resource
        return res;
    }
    // Checks if the stack is empty
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mut);
        return data.empty();
    }
};