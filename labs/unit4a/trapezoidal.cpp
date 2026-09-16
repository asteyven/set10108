#include <cstdio>
#define _USE_MATH_DEFINES
#include <cmath>
#include <thread>
#include <functional>
#include <omp.h>

void trap(double& total_area, std::function<double(double)> func, double x_start, double x_end, int subTrapezoidNum )
{
	auto numThreads = omp_get_num_threads();
	auto thread_id = omp_get_thread_num();
	// what's the x range for each thread? 
	auto step = (x_end - x_start) / double(numThreads);
	// calculate the start x value for this thread
	auto x0 = x_start + thread_id *step;
    double area = 0.0;
	// what's the x range for each trapezoid?
    auto substep = step / subTrapezoidNum;
	// calculate all sub-areas
    auto yprev = func(x0);
    for (int i = 1; i <= subTrapezoidNum; ++i) // start at 1, as we calculate the area as (yprev +y)*h/2
    {
        auto x = x0 + substep*i;
        auto y = func(x);
        area += (yprev + y) * substep * 0.5;
        yprev = y;
    }

#pragma omp critical
	total_area += area;
}

int main(int argc, char** argv)
{
	double total_area = 0.0;
	auto thread_count = std::thread::hardware_concurrency();
#pragma omp parallel num_threads(thread_count)
	trap(total_area, [](double x) {return cos(x); }, 0,M_PI_2, 125);
	printf("Total area: %f\n", total_area);
	return 0;
}