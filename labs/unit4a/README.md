# OpenMP

We are going to move from standard C++ concurrency into a multi-processing support framework - OpenMP.  OpenMP is a mature platform that provides different constructs to standard C++ concurrency. We will investigate these constructs before doing some analysis work.

## First OpenMP Application

First we need to enable OpenMP in our compiler. In Visual Studio you will find an OpenMP option in the project properties (under **C++ --> Language**). You need to enable OpenMP here to ensure that our compiled application is using OpenMP.  For GCC and clang, use the `-fopenmp` compiler flag.  In CMake, we use the following options:

```cmake
find_package(OpenMP)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
```

Our first OpenMP application is a *Hello World* example:

```cpp
#include <iostream>
#include <omp.h>

using namespace std;

// Number of threads to run
constexpr int THREADS = 10;

void hello()
{
    // Get the thread number
    auto my_rank = omp_get_thread_num();
    // Get the number of threads in operation
    auto thread_count = omp_get_num_threads();
    // Display a message
    cout << "Hello from thread " << my_rank << " of " << thread_count << endl;
}

int main(int argc, char **argv)
{
    // Run hello THREADS times
#pragma omp parallel num_threads(THREADS)
    hello();

    return 0;
}
```

The first thing to note is the OpenMP header (`omp.h`). You will need this for some of the OpenMP functions we use, such as getting the thread number (`omp_get_thread_num`) and the number of threads (`omp_get_num_threads`).

`hello` is our operation we are running multiple times. We use a pre-processor directive to tell the compiler that OpenMP code should be generated here. We are running the operation in **`parallel`** - using `num_threads` to tell OpenMP how many copies to run. If you run this application you should get the output below:

```shell
Hello from thread Hello from thread Hello from thread 23 of 6Hello from thread 10 of 10 of 10

7 of 10
Hello from thread 1 of 10
Hello from thread 0 of 10
Hello from thread 8 of 10
Hello from thread 9Hello from thread  of 10
4 of 10
Hello from thread 5 of 10
```

As we are not controlling the output, there is conflict when the threads output at the same time.

## `parallel for`

OpenMP abstracts threads. Threads still exist but are hidden from the developer. One of the constructs OpenMP provides is `parallel for` which allows us to create threads via a `for` loop. Loop iterations are executed in separate threads. You do have to think when using `parallel for` although it can be easier than managing threads.

### Calculating &pi; (not using Monte Carlo Simulation)

Using Monte Carlo simulation to calculate &pi; is good for simulating work, but it is not an efficient method of calculating &pi;. A better method to approximate &pi; is to use the following equation:

![%pi; Calculation](img/pi.png)

We will use this method in a `parallel for` to test `parallel for`.

### Parallel For Implementation of &pi; Approximation

The code below shows OpenMP using a `parallel for`.

```cpp
int main(int argc, char **argv)
{
    // Get number of supported threads
    auto num_threads = thread::hardware_concurrency();

    // Number of iteration values to perform
    const int n = static_cast<int>(pow(2, 30));
    // Factor value
    double factor = 0.0;
    // Calculated pi
    double pi = 0.0;

    // Parallelised for loop that does the work
#pragma omp parallel for num_threads(num_threads) reduction(+:pi) private(factor)
    for (int k = 0; k < n; ++k)
    {
        // Determine sign of factor
        if (k % 2 == 0)
            factor = 1.0;
        else
            factor = -1.0;
        // Add this iteration value to pi sum
        pi += factor / (2.0 * k + 1);
    }

    // Get the final value of pi
    pi *= 4.0;

    // Show more percision of pi
    cout.precision(numeric_limits<double>::digits10);
    cout << "pi = " << pi << endl;

    return 0;
}
```

First, the `parallel for` is similar to a `parallel` but with the keyword `for` added. The two new parts are at the end of the pre-processor statement:

- `reduction` we will cover reduction in more detail later in the module with map-reduce in MPI.  The `pi` variable will be controlled to ensure all the `for` loops add their value.
- `private` indicates that each `for` loop iteration has a private copy of the `factor` value. Each loop iteration can modify the value independently and not cause corruption to another iteration.

If you run this application you will get the output (accurate to 10 decimal places) below:

```shell
pi = 3.14159265265798
```

## Bubble Sort

We will build a parallel sorting mechanism and compare the performance to a bubble sort.  We will build the bubble sort first and you should be familiar with how a bubble sort works.

We need two new functions to support:

1. `generate_values`
2. `bubble_sort`

`parallel_sort` will also use `generate_values`.

### `generate_values`

Below is the function that will generate a `vector` full of values. It simply uses a random engine to do this.

```cpp
// Generates a vector of random values
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
```

### `bubble_sort`

Bubble sort is a straight forward algorithm. We bubble up through the values, swapping them as we go to move a value towards the top.

```psuedo
for count := values.size() to 2 do
    for i := 0 to count - 2 do
        if values[i] > values[i + 1] then
            swap values[i] and values[i + 1]
        end if
    end for
end for
```

### Main Application

Our main application will time the implementation of `bubble_sort` using vectors of different sizes:

```cpp
int main(int argc, char **argv)
{
    // Create results file
    ofstream results("bubble.csv", ofstream::out);
    // Gather results for 2^8 to 2^16 results
    for (size_t size = 8; size <= 16; ++size)
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
            bubble_sort(data);
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
```

If you run this application you should be able to produce a graph as below:

![Bubble Sort Time](img/bubble-sort.png)

However, you should change the y-axis scale to use a `log2` scale to give a straight line.

![Bubble Sort Log Time](img/bubble-sort-log.png)

**Make sure your charts are presented correctly**.  We use the `log2` scale here as our data scales by powers of two.  However, a line chart is incorrect here as the data is not continuous.  A bar or column chart would be better.

## Parallel Sort

Below is a `parallel_sort` taken from *An Introduction to Parallel Programming*. An in-depth description of its development is available in the book (in the OpenMP section).

```cpp
void parallel_sort(vector<unsigned int>& values)
{
    // Get the number of threads
    auto num_threads = thread::hardware_concurrency();
    // Get the number of elements in the vector
    auto n = values.size();
    // Declare the variables used in the loop
    int i, tmp, phase;
    // Declare parallel section
#pragma omp parallel num_threads(num_threads) default(none) shared(values, n) private(i, tmp, phase)
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
                {
                    // Swap values
                    tmp = values[i - 1];
                    values[i - 1] = values[i];
                    values[i] = tmp;
                }
            }
        }
        else
        {
            // Parallel for loop.  Each thread jumps forward 2 so no conflict
#pragma omp for
            for (i = 1; i < n; i += 2)
            {
                // Check is we should swap values
                if (values[i] > values[i + 1])
                {
                    // Swap values
                    tmp = values[i + 1];
                    values[i + 1] = values[i];
                    values[i] = tmp;
                }
            }
        }
    }
}
```

Of more interest is the results.  Normal and logarithmic scaling is provided for comparison.

![Parallel Sort Time](img/parallel-sort.png)

![Parallel Sort Log Time](img/parallel-sort-log.png)

Notice that for small vector sizes parallelism has not provided a performance boost - in fact it is slower. Granted, we are using a different algorithm, but hopefully you understand that the problem set is too small to get any speed up. The set-up and control of the OpenMP program is having an effect. Once our sort space is large enough we gain performance - 3+ times as much.  The CPU is dual core with 4 hardware threads so this is reasonable.

## The Trapezoidal Rule

Our next use of OpenMP will look at something called the trapezoidal rule. This technique can be used to approximate the area under a curve.

![Trapezoidal Rule](img/trapezoidal-rule.png)

We select a number of points on the curve and measure their value. We then use this to generate a number of trapezoids. We can then calculate the area of the trapezoids and get an approximate value for the area under the curve. The more points used on the curve, the better the result.

For our purposes we do not need to worry about why we want to do this - the point is we can parallelise the problem by calculating more trapezoids.

### Trapezoidal Function

The function to work out a section of the area under a curve using the trapezoidal rule is below:

```cpp
void trap(double& total_area, std::function<double(double)> func, double x_start, double x_end, int subTrapezoidNum )
{
	auto numThreads = omp_get_num_threads();
	auto thread_id = omp_get_thread_num();
	auto step = (x_end - x_start) / double(numThreads);
	auto x0 = x_start + thread_id *step;
	
    double area = 0.0;
    auto substep = step / subTrapezoidNum;
    auto yprev = func(x0);
    for (int i = 1; i <= subTrapezoidNum; ++i)
    {
        auto x = x0 + substep*i;
        auto y = func(x);
        area += (yprev + y) * substep * 0.5;
        yprev = y;
    }

#pragma omp critical
	total_area += area;
}
```

The incoming parameters are as follows:

- `total_area` the total area we are going to write to.
- `func` the function we are using to generate the curve.
- `x_start` the starting point of the function's parameter range.
- `x_end` the end point of the function's parameter range.
- `subTrapezoidNum` the number of trapezoids each call to trap() we will generate.

You should be able to follow the algorithm using the comments. The new OpenMP feature is a `critical` section. A `critical` section is a piece of code that only one thread can access at a time - it is controlled by a mutex. We use the `critical` section to control the adding of the local result to the global result.

### Testing the Trapezoidal Algorithm

There is a simple test to check our algorithm using trigonometric functions. For example, the cosine function is:

![Cosine Function](img/cosine.png)

The area under the curve between 0 and &pi; radians is 0 - it is equal parts above and below the line over this period.
The area under the curve between 0 and &pi;/2 radians is 1.

So, by using the cosine function and varying the subTrapezoidNum parameter and the range that we will evaluate the function, since we have the reference integral values, we can evaluate how good the approximation is, by measuring how close we are to the values 0 and 1. Food for thought: Is the area under the 0 and &pi; range ever approximated badly? If not, why not?

## Scheduling

The final concept is scheduling work in OpenMP. Scheduling involves telling OpenMP how to divide work in a `parallel for`. At the moment, each thread is given a chunk of work in order. For example, if we have 1024 iterations and we have 4 threads, our work is divided as follows:

- **Thread 1** iterations 0 to 255.
- **Thread 2** iterations 256 to 511.
- **Thread 3** iterations 512 to 767.
- **Thread 4** iterations 768 to 1023.

For many problems this division works. However, many problems do not divide like this. Scheduling in OpenMP allows us to divide up our work in different manners. Chapter 5 of *Introduction to Parallel Programming* gives more details.

The scheduling method we will use is called `static`. This allows us to allocate work to threads in a round robin manner. For example, a schedule of 1 allocates the work to thread in blocks of 1:

- **Thread 1** 0, 4, 8, 12, ...
- **Thread 2** 1, 5, 9, 13, ...
- **Thread 3** 2, 6, 10, 14, ...
- **Thread 4** 3, 7, 11, 15, ...

Using a schedule of 2 allocates work to threads in blocks of 2:

- **Thread 1** 0, 1, 8, 9, ...
- **Thread 2** 2, 3, 10, 11, ...
- **Thread 3** 4, 5, 12, 13, ...
- **Thread 4** 6, 7, 14, 15, ...

And so on.

### Test Function

Below is a function that can test the effect of scheduling. Work is dependant on the value of `i`.

```cpp
// Let's create a function that relies on i to determine the amount of work
double f(unsigned int i)
{
    // Calculate start and end values
    auto start = i * (i + 1) / 2;
    auto end = start + i;
    // Declare return value
    auto result = 0.0;

    // Loop for number of iterations, calculating sin
    for (auto j = start; j <= end; ++j)
        result += sin(j);

    // Return result
    return result;
}
```

### Main Application

Below is our test application. We use the schedule function in the pre-processor argument to control the division of work. Your task here is to manipulate the schedule value and see the effect.

```cpp
int main(int argc, char **argv)
{
    // Get number of hardware threads
    auto thread_count = thread::hardware_concurrency();
    // Define number of iterations to calculate
    auto n = static_cast<size_t>(pow(2, 14));
    // Declare sum value
    auto sum = 0.0;

    // Get start time
    auto start = system_clock::now();
#pragma omp parallel for num_threads(thread_count) reduction(+:sum) schedule(static, 1)
    for (auto i = 0; i <= n; ++i)
        sum += f(i);
    // Get end time
    auto end = system_clock::now();
    // Calculate and output total time
    auto total = duration_cast<milliseconds>(end - start).count();
    cout << "Total time: " << total << "ms" << endl;

    return 0;
}
```

Running this application will output a timing value - test the scheduling value and chart the difference in performance.

## Exercises

1. Try using the schedule technique to split up the work for the Mandelbrot fractal.  This will allow more control over how work is divided. You need to understand where the likely bottlenecks are in the algorithm in relation to the image produced to work out how best to split Mandelbrot up for OpenMP.
2. You now have enough information to build a queue to act as a message passing interface. Build one - either using standard C++ threading, OpenMP or both - and show it works using a basic producer-consumer model.

## Reading

You should be reading Chapter 5 of Introduction to Parallel Programming for more information on OpenMP.
