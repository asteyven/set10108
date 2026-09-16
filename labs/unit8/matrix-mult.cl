__kernel void simple_multiply(__global float *output_C, unsigned int width_A, unsigned int height_A, unsigned int width_B, unsigned int height_B, __global float *input_A, __global float *input_B)
{
    // Get global position in Y direction
    unsigned int row = get_global_id(1);
    // Get global position in X direction
    unsigned int col = get_global_id(0);

    float sum = 0.0f;

    // Calculate result of one element of matrix C
    for (unsigned int i = 0; i < width_A; ++i)
        sum += input_A[row * width_A + i] * input_B[i * width_B + col];

    // Store result in matrix C
    output_C[row * width_B + col] = sum;
}