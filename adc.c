#include "adc.h"
#include "stats.h"
#include <stdlib.h>
#include <float.h>

// Convert raw ADC value to voltage (0-3.3V)
double convert_voltage(uint16_t raw_value)
{
    return ((double)raw_value / ADC_MAX_COUNT) * VREF;
}

// Calculate stats and detect faults for each channel
void compute_channel_stats(const ADCSample *records, uint32_t count, ChannelStats stats[NUM_CHANNELS])
{
    int ch;

    // Initialize stats array
    for (ch = 0; ch < NUM_CHANNELS; ch++) {
        stats[ch].channel_id = ch;
        stats[ch].mean = 0.0;
        stats[ch].rms = 0.0;
        stats[ch].min = DBL_MAX;
        stats[ch].max = -DBL_MAX;
        stats[ch].std_dev = 0.0;
        stats[ch].count = 0;
        stats[ch].overvoltage_count = 0;
        stats[ch].undervoltage_count = 0;
        stats[ch].fault_count = 0;
        stats[ch].oor_count = 0;
    }

    // Pass 1: count records per channel to allocate memory
    const ADCSample *p = records;
    for (uint32_t i = 0; i < count; i++) {
        if (p->channel_id < NUM_CHANNELS) {
            stats[p->channel_id].count++;
        }
        p++;
    }

    // Pass 2: extract voltages and calculate math
    for (ch = 0; ch < NUM_CHANNELS; ch++) {
        int n = stats[ch].count;
        if (n == 0) continue;

        double *voltages = (double *)malloc((size_t)n * sizeof(double));
        if (voltages == NULL) {
            stats[ch].count = 0;
            continue; // Skip if memory allocation fails
        }

        double *vp = voltages;
        p = records;
        
        for (uint32_t i = 0; i < count; i++) {
            if ((int)p->channel_id == ch) {
                double v = p->voltage;
                *vp = v;
                vp++;

                // Check voltage thresholds
                if (v > OVERVOLTAGE_LIMIT)  
                    stats[ch].overvoltage_count++;
                if (v < UNDERVOLTAGE_LIMIT) 
                    stats[ch].undervoltage_count++;

                // Check status flags (bit 0 = fault, bit 1 = out of range)
                if (p->status_flags & 0x01) 
                    stats[ch].fault_count++;
                if (p->status_flags & 0x02) 
                    stats[ch].oor_count++;
            }
            p++;
        }

        // Compute final statistics
        stats[ch].mean    = stats_mean(voltages, n);
        stats[ch].rms     = stats_rms(voltages, n);
        stats[ch].min     = stats_min(voltages, n);
        stats[ch].max     = stats_max(voltages, n);
        stats[ch].std_dev = stats_std_dev(voltages, n);

        free(voltages);
        voltages = NULL;
    }
}

// Find dropped samples by checking sequence numbers
int check_sequence_integrity(const ADCSample *records, uint32_t count, SequenceGap *gaps, int max_gaps)
{
    if (count < 2 || gaps == NULL || max_gaps <= 0) return 0;

    int gap_count = 0;
    const ADCSample *current = records;       
    const ADCSample *next = records + 1;   

    for (uint32_t i = 0; i < count - 1; i++) {
        uint32_t expected = current->sequence_number + 1;

        // If next sequence number is higher than expected, we dropped data
        if (next->sequence_number > expected) {
            if (gap_count < max_gaps) {
                SequenceGap *g = gaps + gap_count;
                g->expected_seq = expected;
                g->found_seq    = next->sequence_number;
                g->gap_size     = (int)(next->sequence_number - expected);
                g->timestamp    = next->timestamp;
            }
            gap_count++;
        }
        current++;   
        next++;
    }

    return gap_count;
}
