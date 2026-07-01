#include <stdio.h>
#include <stdlib.h>
#include "adc.h"
#include "io.h"

#define MAX_GAPS 64

int main(int argc, char *argv[])
{
    // Check command line arguments
    if (argc != 2) {
        fprintf(stderr, "Hey, you need to provide the binary file path!\n");
        fprintf(stderr, "Usage: %s <binary_file.bin>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_file = argv[1];
    const char *output_file = "results.txt";

    // Read binary file header and records
    FileHeader header;
    ADCSample *records = read_binary_file(input_file, &header);
    if (records == NULL) {
        return EXIT_FAILURE;
    }
    printf("Successfully loaded %u records into memory.\n", header.record_count);

    // Calculate statistics
    ChannelStats stats[NUM_CHANNELS];
    compute_channel_stats(records, header.record_count, stats);
    printf("Finished computing stats for all %d channels.\n", NUM_CHANNELS);

    // Check for dropped samples
    SequenceGap gaps[MAX_GAPS];
    int gap_count = check_sequence_integrity(records, header.record_count, gaps, MAX_GAPS);
    printf("Checked sequences: Found %d gap(s) in the data.\n", gap_count);

    // Write results to text file
    if (write_results(output_file, &header, stats, gaps, gap_count) != 0) {
        free(records);
        return EXIT_FAILURE;
    }

    // Free heap memory to prevent leaks
    free(records);
    records = NULL;

    printf("All done! I've written the full report out to '%s'.\n", output_file);
    return EXIT_SUCCESS;
}
