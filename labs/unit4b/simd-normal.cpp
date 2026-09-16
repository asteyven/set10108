#include <iostream>
#include <random>
#include <chrono>
#include <immintrin.h>

using namespace std;
using namespace std::chrono;

union v4
{
    __m128 v;    // SSE 4 x float vector
    float a[4];  // scalar array of 4 floats
};

// Randomly generate vector values
void generate_data(v4 *data, size_t n)
{
    // Create random engine
    random_device r;
    default_random_engine e(r());
    // Fill data
    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < 4; ++j)
            data[i].a[j] = e();
}

// Normalises the vector
void normalise_vectors(v4 *data, v4 *result, size_t n)
{
    // Normalise the vectors
    for (size_t i = 0; i < n; ++i)
    {
        // Square each component - simply multiply the vectors by themselves
        result[i].v = _mm_mul_ps(data[i].v, data[i].v);
        // Calculate sum of the components.
        // See notes to explain hadd.
        result[i].v = _mm_hadd_ps(result[i].v, result[i].v);
        result[i].v = _mm_hadd_ps(result[i].v, result[i].v);
        // Calculate recipricol square root of the values
        // That is 1.0f / sqrt(value) - or the length of the vector
        result[i].v = _mm_rsqrt_ps(result[i].v);
        // Multiply result by the original data
        // As we have the recipricol it is the same as dividing each component
        // by the length
        result[i].v = _mm_mul_ps(data[i].v, result[i].v);
    }
    // All vectors now normalised
}

// Check the first 100 results
void check_results(v4 *data, v4 *result)
{
    // Check first 100 values
    for (size_t i = 0; i < 100; ++i)
    {
        // Calculate the length of the vector
        float l = 0.0f;
        // Square each component and add to l
        for (size_t j = 0; j < 4; ++j)
            l += powf(data[i].a[j], 2.0f);
        // Get sqrt of the length
        l = sqrtf(l);
        // Now check that the individual results
        for (size_t j = 0; j < 4; ++j)
            cout << data[i].a[j] / l << " : " << result[i].a[j] << endl;
    }
}

int main(int argc, char **argv)
{
    v4 *data = (v4*)_aligned_malloc(sizeof(v4) * 1000000, 16);
    v4 *result = (v4*)_aligned_malloc(sizeof(v4) * 1000000, 16);
    generate_data(data, 1000000);
    normalise_vectors(data, result, 1000000);
    check_results(data, result);
    _aligned_free(data);
    _aligned_free(result);
    return 0;
}