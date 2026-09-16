__kernel void image_rotate(__global int *dest_data, __global int *src_data, unsigned int width, unsigned int height, float sin_theta, float cos_theta)
{
    // Get work item index - (x, y) coordinate
    const int ix = get_global_id(0);
    const int iy = get_global_id(1);

    // Calculate location of data
    float x0 = (float)width / 2.0f;
    float y0 = (float)height / 2.0f;
    float xOff = ix - x0;
    float yOff = iy - y0;
    int xpos = (int)(xOff * cos_theta + yOff * sin_theta + x0);
    int ypos = (int)(yOff * cos_theta - xOff * sin_theta + y0);

    // Bounds check - should we set some data?
    if ((xpos >= 0) && (xpos < width) && (ypos >= 0) && (ypos < height))
        dest_data[iy * width + ix] = src_data[ypos * width + xpos];
}