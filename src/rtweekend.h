#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>

// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

thread_local unsigned int SEED = 42;  // each thread has own copy

inline double random_double() {
    // rand_r is thread_safe and uses SEED to start and keep track of sequence
    double val = 1.0 * rand_r(&SEED) / (RAND_MAX + 1.0);
    return val;
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}

#endif
