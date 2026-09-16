__kernel void monte_carlo_pi(__global float2 *points, __local char *local_results, __global int *global_results)
{
    // Get our id
    unsigned int global_id = get_global_id(0);
    unsigned int local_id = get_local_id(0);
    unsigned int local_dim = get_local_size(0);
    unsigned int group_id = get_group_id(0);
    
    // Get the point to work on
    float2 point = points[global_id];
    // Calculate the length - built-in OpenCL function
    float l = length(point);
    // Result is either 1 or 0
    if (l <= 1.0f)
        local_results[local_id] = 1;
    else
        local_results[local_id] = 0;

    // Wait for the entire work group to get here.
    barrier(CLK_LOCAL_MEM_FENCE);

    // If work item 0 in work group sum local values
    if (local_id == 0)
    {
        int sum = 0;
        for (int i = 0; i < local_dim; ++i)
        {
            if (local_results[i] == 1)
                ++sum;
        }
        global_results[group_id] = sum;
    }
}