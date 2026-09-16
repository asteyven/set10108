#include <iostream>
#include <vector>
#include <future>

using namespace std;

// Number of iterations to perform to find pixel value
constexpr size_t max_iterations = 1000;

// Dimension of the image (in pixels) to generate
constexpr size_t dim = 8192;

// Mandelbrot dimensions are ([-2.1, 1.0], [-1.3, 1.3])
constexpr double xmin = -2.1;
constexpr double xmax = 1.0;
constexpr double ymin = -1.3;
constexpr double ymax = 1.3;

// The conversion from Mandelbrot coordinate to image coordinate
constexpr double integral_x = (xmax - xmin) / static_cast<double>(dim);
constexpr double integral_y = (ymax - ymin) / static_cast<double>(dim);

vector<double> mandelbrot(size_t start_y, size_t end_y)
{
    // Declare values we will use
    double x, y, x1, y1, xx = 0.0;
    size_t loop_count = 0;
    // Where to store the results
    vector<double> results;

    // Loop through each line
    y = ymin + (start_y * integral_y);
    for (size_t y_coord = start_y; y_coord < end_y; ++y_coord)
    {
        x = xmin;
        // Loop through each pixel on the line
        for (size_t x_coord = 0; x_coord < dim; ++x_coord)
        {
            x1 = 0.0, y1 = 0.0;
            loop_count = 0;
            // Calculate Mandelbrot value
            while (loop_count < max_iterations && sqrt((x1 * x1) + (y1 * y1)) < 2.0)
            {
                ++loop_count;
                xx = (x1 * x1) - (y1 * y1) + x;
                y1 = 2 * x1 * y1 + y;
                x1 = xx;
            }
            // Get value where loop completed
            auto val = static_cast<double>(loop_count) / static_cast<double>(max_iterations);
            // Push this value onto the vector
            results.push_back(val);
            // Increase x based on integral
            x += integral_x;
        }
        // Increase y based on integral
        y += integral_y;
    }
    // Return vector
    return results;
}

int main(int agrc, char **argv)
{
    // Get the number of supported threads
    auto num_threads = thread::hardware_concurrency();

    // Determine strip height
    size_t strip_height = dim / num_threads;

    // Create futures
    vector<future<vector<double>>> futures;
    for (unsigned int i = 0; i < num_threads; ++i)
        // Range is used to determine number of values to process
        futures.push_back(async(mandelbrot, i * strip_height, (i + 1) * strip_height));

    // Vector to store results
    vector<vector<double>> results;
    // Get results
    for (auto &f : futures)
        results.push_back(f.get());

    return 0;
}