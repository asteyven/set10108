# GPU Exercises

Today you'll practice creating your own programs to achieve the given tasks. Clone the base project to have a base to begin with:

1. Copy the CMakeLists.txt from another lab folder (unit2 if CUDA, unit4 if OpenCL) here 
2. Edit the CMakeLists.txt file, remove all sources and replace them with your source files, which can start as the contents of vector addition.
3. Configure and Generate in CMake, and start programming

## Task 1: 1D smoothing

1. Create a vector of N random float values (between 0 and 1)
2. Write a GPU kernel that applies smoothing to the values using an 1D Gaussian kernel (calculator [here](https://web.archive.org/web/20220916052214/http://dev.theomader.com/gaussian-kernel-calculator/))
3. Allow arbitrary kernel radius (remember that you'll need (2\*radius + 1) kernel elements)
4. Visualise the results, by printing or otherwise.

You have to be careful with the bounds. You can try and use local memory for intermediate results, as shown in the lecture.

## Task 2: 2D smoothing

1. Create a vector of MxN random float values (between 0 and 1), or use a grayscale image. You can use stb_image to load images.
2. Write a GPU kernel that applies smoothing to the values using a 2D Gaussian kernel (calculator [here](https://web.archive.org/web/20220916052214/http://dev.theomader.com/gaussian-kernel-calculator/))
3. Allow arbitrary kernel radius (remember that you'll need (2\*radius + 1)<sup>2</sup> kernel elements)
4. Visualise the results. You can convert the results to an image using stb_image.

As before, you have to be careful with the bounds. You can try and use local memory for intermediate results, as shown in the lecture. This is much harder than the 1D version!
