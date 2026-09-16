#include <cstdio>
#include <cmath>
#include <thread>
#include <vector>
#include <algorithm>
#include <future>

#include <unistd.h>
#include <limits.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "cxxopts.hpp"

using namespace std;

// Number of iterations to perform to find pixel value
constexpr size_t max_iterations = 1000;

// Mandelbrot dimensions are ([-2.1, 1.0], [-1.3, 1.3])
// Adjust it to be square (for convenience), by expanding the y coordinates
constexpr double xmin_global = -2.1;
constexpr double xmax_global = 1.0;
constexpr double extent_global = xmax_global - xmin_global;
constexpr double ymin_global = -extent_global/2;
constexpr double ymax_global = extent_global/2;

vector<uint8_t> mandelbrot(size_t start_y, size_t end_y, int dim, int tile_id, int tiles_per_row_col)
{
    const auto tile_id_x = tile_id % tiles_per_row_col;
    const auto tile_id_y = tile_id / tiles_per_row_col;
    const double extent_tile = extent_global/tiles_per_row_col;
    const double xmin = xmin_global + tile_id_x*extent_tile;
    const double ymin = ymin_global + tile_id_y*extent_tile;
    
    // The conversion from Mandelbrot coordinate to image coordinate
    const double integral_x = extent_tile / dim;
    const double integral_y = extent_tile / dim;

    // Declare values we will use
    double x, y, x1, y1, xx = 0.0;
    size_t loop_count = 0;
    // Where to store the resulting RGB data. Pre-allocate, as the size is known
    vector<uint8_t> results(dim * (end_y - start_y) * 3);

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
            double sqlen = x1 * x1 + y1 * y1;
            while (loop_count < max_iterations && sqlen < 512.0)
            {
                ++loop_count;
                xx = (x1 * x1) - (y1 * y1) + x;
                y1 = 2 * x1 * y1 + y;
                x1 = xx;
                sqlen = x1 * x1 + y1 * y1;
            }
            
            // generate color from loop count
            auto smooth_loop_count = loop_count - log2(log2(sqlen)) + 4.0;
            auto red = 0.5 + 0.5*cos( 3.0 + smooth_loop_count*0.15 + 0.0);
            auto green = 0.5 + 0.5*cos( 3.0 + smooth_loop_count*0.15 + 0.6);
            auto blue = 0.5 + 0.5*cos( 3.0 + smooth_loop_count*0.15 + 1.0);
            
            // assign color to results
            auto idx = (y_coord-start_y)*dim + x_coord;
            results[idx*3] = uint8_t(red*255);
            results[idx*3+1] = uint8_t(green*255);
            results[idx*3+2] = uint8_t(blue*255);
            
            // Increase x based on integral
            x += integral_x;
        }
        // Increase y based on integral
        y += integral_y;
    }
    // Return vector
    return results;
}

int main(int argc, char ** argv)
{    
    
    //testparse(argc,argv);
    //return 0;
    // Parse command line arguments
    cxxopts::Options options("Mandelbrot set tile generator", 
                             "Evaluates the Mandelbrot set in a square region");
                             
    options.add_options()
      ("o,output", "Output PNG format (e.g. /path/to/myimage%02.png)", cxxopts::value<string>())
      ("N,num", "Number of tile rows/columns", cxxopts::value<int>())
      ("s,size", "Size of square tile in pixels", cxxopts::value<int>())
      ("j,job", "Job array id, in [0,N*N-1]", cxxopts::value<int>())
      ;
     
    auto result = options.parse(argc, argv);
    
    auto outputImageFilename = result["output"].as<string>();
    auto dim = result["size"].as<int>();
    auto tiles_per_row_col = result["num"].as<int>();
    auto tile_id = result["job"].as<int>();
    

    //may return 0 when not able to detect
    // Get the number of supported threads
    const auto num_threads = thread::hardware_concurrency();
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);

    // Determine strip height. 
    // size_t strip_height = dim / num_threads;
    // E.g. if 64 px / 12 threads: what happens? strip_height will round down to 5, and 5*12 = 60 => we will NOT cover all required pixels!
    // The below will result in just enough: (64+11) / 12
    size_t strip_height = (dim + num_threads-1) / num_threads; 

    // Create futures
    vector<future<vector<uint8_t>>> futures;
    for (unsigned int i = 0; i < num_threads; ++i)
        // Range is used to determine number of values to process
        futures.push_back(async(mandelbrot, i * strip_height, (i + 1) * strip_height, dim, tile_id, tiles_per_row_col));
    
    // Vector to store results
    vector<uint8_t> results(dim*dim*3);
    // Get results
    auto it_out = results.begin();
    for (auto &f : futures)
    {
        const auto fdata = f.get();
        std::copy(fdata.begin(), fdata.end(), it_out);
        it_out += fdata.size();
    }
    
    char filename[512];
    sprintf(filename, outputImageFilename.c_str(), tile_id);
    stbi_write_png(filename, dim, dim, 3, results.data(), dim*3);
    return 0;
}