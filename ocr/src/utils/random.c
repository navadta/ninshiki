#include "utils/random.h"

#include <math.h>
#include <stdlib.h>

double random_double(double lower, double upper) {
    int rnd = rand();
    double scale = upper - lower;
    return lower + (rnd * scale / RAND_MAX);
}

double normal_distribution(double mean, double stddev) {
    static unsigned char has_spare = 0;
    static double spare;

    if (has_spare) {
        has_spare = 0;
        return mean + stddev * spare;
    }

    has_spare = 1;

    static double u, v, s;

    do {
        u = (rand() * 2.0 / RAND_MAX) - 1.0;
        v = (rand() * 2.0 / RAND_MAX) - 1.0;
        s = u * u + v * v;
    } while (s >= 1.0 || s == 0.0);

    s = sqrt(-2.0 * log(s) / s);
    spare = v * s;

    return mean + stddev * u * s;
}
