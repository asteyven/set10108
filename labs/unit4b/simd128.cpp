#include <iostream>
#include <immintrin.h>

using namespace std;

// Union can flip between the types within.
union v4
{
    __m128 v;    // SSE 4 x float vector
    float a[4];  // scalar array of 4 floats
};

int main(int argc, char **argv)
{
    // Declare a single 128-bit value aligned to 16 bytes (size of 128-bits)
	alignas(16) v4 x;
	// We can treat x as a collection of four floats
	// Or other combinations of values for 128-bits
	x.a[0] = 10.0f;
	x.a[1] = 20.0f;
	x.a[2] = 30.0f;
	x.a[3] = 40.0f;
	// We can print out individual values
    cout << "Original values" << endl;
    for (size_t i = 0; i < 4; ++i)
	    cout << x.a[i] << endl;

    // Declared a second 128-bit value alignted to 16 bytes (size of 128-bits)
    alignas(16) v4 y;
    y.a[0] = 10.0f;
    y.a[1] = 20.0f;
    y.a[2] = 30.0f;
    y.a[3] = 40.0f;
    // Add y to x
    x.v = _mm_add_ps(x.v, y.v);
    // Print out individual values
    cout << "New values" << endl;
    for (size_t i = 0; i < 4; ++i)
	    cout << x.a[i] << endl;

    // Create array of 100 floats, aligned to 4 bytes.
    float *data = (float*)_aligned_malloc(100 * sizeof(float), 4);
    // Access just like an array
	cout << data[0] << endl;

    // Create an array of 100 128-bit values aligned to 16 bytes
	v4 *big_data = (v4*)_aligned_malloc(100 * sizeof(v4), 16);

	// Access just like an array of __m128
	cout << big_data[0].a[0] << endl;

	// Free the data - ALWAYS REMEMBER TO FREE YOUR MEMORY
	// We are dealing at a C level here
	_aligned_free(data);
	_aligned_free(big_data);

    return 0;
}