#ifndef IO_H
#define IO_H

#include "adc.h"
#include <stdint.h>

// Read the binary log file and return an array of ADCSamples on the heap
ADCSample *read_binary_file(const char *filename, FileHeader *out_header);

// Write the analysis results to a text file
int write_results(const char *filename, const FileHeader *header, const ChannelStats stats[NUM_CHANNELS], const SequenceGap *gaps, int gap_count);

#endif /* IO_H */
