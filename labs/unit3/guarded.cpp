#include "guarded.h"

void guarded::increment()
{
    std::lock_guard<std::mutex> lock(mut);
    int x = value;
    x = x + 1;
    value = x;
}