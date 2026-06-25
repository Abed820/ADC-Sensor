#ifndef STATS_H
#define STATS_H

// Math utility functions for double arrays
double stats_mean(const double *data, int n);
double stats_min(const double *data, int n);
double stats_max(const double *data, int n);
double stats_std_dev(const double *data, int n);
double stats_rms(const double *data, int n);

#endif /* STATS_H */
