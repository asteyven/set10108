// This kernel is adapted from Ben Hiller's code on Github
// https://github.com/benhiller/opencl-mandelbrot

// Convert the raw x coordinates [0, 1] into a scaled coordinate
// [0, 1] -> [-2, 1.25]
float mapX(float x)
{
    return x * 3.25f - 2.0f;
}

// Same purpose as mapX
// [0, 1] -> [-1.25, 1.25]
float mapY(float y)
{
    return y * 2.5f - 1.25f;
}

__kernel void mandelbrot(__global char *out)
{
    int x_dim = get_global_id(0);
    int y_dim = get_global_id(1);
    size_t width = get_global_size(0);
    size_t height = get_global_size(1);
    int idx = width * y_dim + x_dim;
    
    float x_origin = mapX((float)x_dim / (float)width);
    float y_origin = mapY((float)y_dim / (float)height);
    
    // Escape time algorithm.  Follows the pseudocode from Wikipedia _very_ closely
    float x = 0.0f;
    float y = 0.0f;
    
    int iteration = 0;
    
    // This can be changed to be more or less precise
    int max_iteration = 256;
    
    // While loop - is this the best option???
    while (x * x + y * y <= 4 && iteration < max_iteration)
    {
        float xtemp = x * x - y * y + x_origin;
        y = 2 * x * y + y_origin;
        x = xtemp;
        ++iteration;
    }
    
    if (iteration == max_iteration)
        // This coordinate did not escape, so is in the Madelbrot set
        out[idx] = 0;
    else
        // This coordinate did escape, so colour based on how quickly it escaped
        out[idx] = iteration;
}