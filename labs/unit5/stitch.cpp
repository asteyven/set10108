#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "cxxopts.hpp"

using namespace std;

int main(int argc, char ** argv)
{   
    // Parse command line arguments
    cxxopts::Options options("Tile image stitcher", 
                             "Stitches PNG tile images arranged in a NxN configuration, and outputs the result to a new PNG image");
    options.add_options()
      ("i,input", "Input format (e.g. /path/to/myimage%02.png)", cxxopts::value<string>())
      ("o,output", "Output file name (png)", cxxopts::value<string>())
      ("N,num", "Number of tile rows/columns", cxxopts::value<int>())
      ;
    auto result = options.parse(argc, argv);
    auto outputImageFilename = result["output"].as<string>();
    auto inputImageFormat = result["input"].as<string>();
    auto tilesPerRowCol = result["num"].as<int>();
    
    // allocate storage for image filenames
    char inputImageFilename[512];
    
    // load all images
    int w,h,c,pitch; // width, height, components and pitch of image (size in bytes of a row in the input image)
    int wout, hout;
    vector<unsigned char> img_out;
    for(int tile_y=0; tile_y < tilesPerRowCol; ++tile_y)
        for(int tile_x=0; tile_x < tilesPerRowCol; ++tile_x)
        {
            int tile_idx = tile_x + tile_y*tilesPerRowCol;
            sprintf(inputImageFilename, inputImageFormat.c_str(), tile_idx);
            auto img_in = stbi_load(inputImageFilename, &w, &h,&c, 0);
            if(img_out.empty())
            {
                pitch = w*c;
                wout = w*tilesPerRowCol;
                hout = h*tilesPerRowCol;
                img_out.resize(wout * hout * c); // allocate storage for output image
            }
            
            int xout = tile_x * w;
            int yout = tile_y * h;
            for(int row = 0; row < h; ++row)
            {
                int pixel_offset_out = (xout + (yout + row)*wout)*c;
                int pixel_offset_in = row*pitch;
                memcpy(img_out.data() + pixel_offset_out, img_in + pixel_offset_in, pitch); // copy a single line
            }
            
            stbi_image_free(img_in);
        }
    
    stbi_write_png(outputImageFilename.c_str(), wout, hout, c, img_out.data(), wout*c);
    
    return 0;
}