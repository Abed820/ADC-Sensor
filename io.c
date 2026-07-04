#include "io.h"
#include "adc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ADC_MAGIC  0xADC1BEEF

// Reads binary file and dynamically allocates memory for records
ADCSample *read_binary_file(const char *filename, FileHeader *out_header)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Oops: Couldn't open the file '%s'. Does it exist?\n", filename);
        return NULL;
    }

    // Read the 24-byte header
    size_t items_read = fread(out_header, sizeof(FileHeader), 1, fp);
    if (items_read != 1) {
        fprintf(stderr, "Error: Failed to read file header.\n");
        fclose(fp);
        return NULL;
    }

    // Validate magic number
    if (out_header->magic != ADC_MAGIC) {
        fprintf(stderr, "Error: Invalid magic number 0x%08X\n", out_header->magic);
        fclose(fp);
        return NULL;
    }

    uint32_t count = out_header->record_count;
    
    // Allocate memory for all records
    ADCSample *records = (ADCSample *)malloc(count * sizeof(ADCSample));
    RawRecord *raw_records = (RawRecord *)malloc(count * sizeof(RawRecord));
    if (records == NULL || raw_records == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for %u records.\n", count);
        fclose(fp);
        if (records) free(records);
        if (raw_records) free(raw_records);
        return NULL;
    }

    // Read all records in one go
    size_t records_read = fread(raw_records, sizeof(RawRecord), count, fp);
    if (records_read != (size_t)count) {
        fprintf(stderr, "Warning: Expected %u records but found %zu.\n", count, records_read);
        out_header->record_count = (uint32_t)records_read;
    }

    // Convert raw records to ADCSample and calculate voltage
    for (size_t i = 0; i < records_read; i++) {
        records[i].timestamp = raw_records[i].timestamp;
        records[i].channel_id = raw_records[i].channel_id;
        records[i].raw_value = raw_records[i].raw_value;
        records[i].voltage = convert_voltage(raw_records[i].raw_value);
        records[i].temperature = raw_records[i].temperature;
        records[i].status_flags = raw_records[i].status_flags;
        records[i].sequence_number = raw_records[i].sequence_number;
    }

    free(raw_records);
    fclose(fp);
    return records;
}

// Writes the final report to results.txt
int write_results(const char *filename, const FileHeader *header, const ChannelStats stats[NUM_CHANNELS], const SequenceGap *gaps, int gap_count)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not create output file '%s'\n", filename);
        return -1;
    }

    // Get current time
    time_t now = time(NULL);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // Print Report Header
    fprintf(fp,
        "================================================================================\n"
        "  ADC SENSOR LOG ANALYSIS REPORT\n"
        "  Generated on: %s\n"
        "================================================================================\n\n",
        time_buf);

    fprintf(fp,
        "FILE INFORMATION\n"
        "--------------------------------------------------------------------------------\n"
        "  Magic number   : 0x%08X\n"
        "  File version   : %u\n"
        "  Channel count  : %u\n"
        "  Record count   : %u\n"
        "  Sample rate    : %u Hz\n\n",
        header->magic, header->version, header->channel_count, header->record_count, header->sample_rate_hz);

    const char *channel_desc[NUM_CHANNELS] = {
        "CH0 - 5 Hz sine, centred 1.65 V, clean reference",
        "CH1 - 3 Hz sine, centred 1.50 V, overvoltage spikes",
        "CH2 - 7 Hz sine, centred 1.80 V, undervoltage drops",
        "CH3 - 2 Hz sine, centred 1.40 V, fault flags"
    };

    // Print Statistics
    fprintf(fp, "PER-CHANNEL VOLTAGE STATISTICS\n--------------------------------------------------------------------------------\n");
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        const ChannelStats *s = &stats[ch];
        fprintf(fp,
            "\n  %s\n"
            "  Samples     : %d\n  Mean        : %9.6f V\n  RMS         : %9.6f V\n"
            "  Minimum     : %9.6f V\n  Maximum     : %9.6f V\n  Std Dev     : %9.6f V\n",
            channel_desc[ch], s->count, s->mean, s->rms, s->min, s->max, s->std_dev);
    }

    // Print Faults
    fprintf(fp,
        "\n\nFAULT DETECTION SUMMARY\n--------------------------------------------------------------------------------\n"
        "  %-6s  %-14s  %-14s  %-14s  %-10s\n  %-6s  %-14s  %-14s  %-14s  %-10s\n",
        "Chan", "Overvoltage", "Undervoltage", "Sensor Fault", "OOR",
        "------", "--------------", "--------------", "--------------", "----------");

    int total_ov = 0, total_uv = 0, total_fault = 0, total_oor = 0;
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        const ChannelStats *s = &stats[ch];
        fprintf(fp, "  CH%-4d  %-14d  %-14d  %-14d  %-10d\n",
            ch, s->overvoltage_count, s->undervoltage_count, s->fault_count, s->oor_count);
        total_ov += s->overvoltage_count;
        total_uv += s->undervoltage_count;
        total_fault += s->fault_count;
        total_oor += s->oor_count;
    }
    
    fprintf(fp,
        "  %-6s  %-14s  %-14s  %-14s  %-10s\n  %-6s  %-14d  %-14d  %-14d  %-10d\n",
        "------", "--------------", "--------------", "--------------", "----------",
        "TOTAL", total_ov, total_uv, total_fault, total_oor);

    // Print Sequence Integrity
    fprintf(fp, "\n\nSEQUENCE INTEGRITY CHECK\n--------------------------------------------------------------------------------\n");
    if (gap_count == 0) {
        fprintf(fp, "  No sequence gaps found.\n");
    } else {
        fprintf(fp, "  Found %d sequence gap(s):\n\n", gap_count);
        fprintf(fp,
            "  %-6s  %-14s  %-14s  %-12s  %-12s\n  %-6s  %-14s  %-14s  %-12s  %-12s\n",
            "Gap#", "Expected Seq", "Found Seq", "Missing", "At Time(s)",
            "----", "------------", "---------", "-------", "---------");

        for (int i = 0; i < gap_count; i++) {
            const SequenceGap *g = gaps + i;
            fprintf(fp, "  %-6d  %-14u  %-14u  %-12d  %.4f\n",
                i + 1, g->expected_seq, g->found_seq, g->gap_size, g->timestamp);
        }
    }

    fprintf(fp, "\n================================================================================\n  END OF REPORT\n================================================================================\n");

    fclose(fp);
    return 0;
}
