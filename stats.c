#include "stats.h"
#include <math.h>

// Calculate arithmetic mean
double stats_mean(const double *data, int n)
{
    if (n <= 0) return 0.0;

    double sum = 0.0;
    const double *p = data;
    for (int i = 0; i < n; i++) {
        sum += *p;
        p++;
    }
    return sum / (double)n;
}

// Find minimum value
double stats_min(const double *data, int n)
{
    if (n <= 0) return 0.0;

    double minimum = *data;
    const double *p = data + 1;
    for (int i = 1; i < n; i++) {
        if (*p < minimum) {
            minimum = *p;
        }
        p++;
    }
    return minimum;
}

// Find maximum value
double stats_max(const double *data, int n)
{
    if (n <= 0) return 0.0;

    double maximum = *data;
    const double *p = data + 1;
    for (int i = 1; i < n; i++) {
        if (*p > maximum) {
            maximum = *p;
        }
        p++;
    }
    return maximum;
}

// Calculate standard deviation using a two-pass algorithm to avoid precision loss
double stats_std_dev(const double *data, int n)
{
    if (n < 2) return 0.0;

    double mean = stats_mean(data, n);
    double sum_sq = 0.0;
    const double *p = data;
    
    for (int i = 0; i < n; i++) {
        double diff = *p - mean;
        sum_sq += diff * diff;
        p++;
    }

    return sqrt(sum_sq / (double)n);
}

// Calculate root mean square (RMS)
double stats_rms(const double *data, int n)
{
    if (n <= 0) return 0.0;

    double sum_sq = 0.0;
    const double *p = data;
    for (int i = 0; i < n; i++) {
        sum_sq += (*p) * (*p);
        p++;
    }
    
    return sqrt(sum_sq / (double)n);
}
