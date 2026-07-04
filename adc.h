#ifndef ADC_H
#define ADC_H

#include <stdint.h>

#define NUM_CHANNELS 4
#define ADC_MAX_COUNT 4095.0
#define VREF 3.3
#define OVERVOLTAGE_LIMIT 3.0
#define UNDERVOLTAGE_LIMIT 0.3

// Ensure structs match exact binary layout
#pragma pack(push, 1)

// File header (24 bytes)
typedef struct {
    uint32_t magic;          
    uint16_t version;        
    uint16_t channel_count;  
    uint32_t record_count;   
    uint32_t sample_rate_hz; 
    uint8_t  reserved[8];    
} FileHeader;

// Single ADC reading on disk (16 bytes)
typedef struct {
    float    timestamp;       
    uint8_t  channel_id;      
    uint16_t raw_value;       
    int16_t  temperature;     
    uint8_t  status_flags;    
    uint32_t sequence_number; 
    uint8_t  reserved[2];     
} RawRecord;

#pragma pack(pop)

// In-memory record with computed voltage
typedef struct {
    float    timestamp;       
    uint8_t  channel_id;      
    uint16_t raw_value;
    double   voltage;
    int16_t  temperature;     
    uint8_t  status_flags;    
    uint32_t sequence_number; 
} ADCSample;

// Stats for a single channel
typedef struct {
    int channel_id;
    double mean;
    double rms;
    double min;
    double max;
    double std_dev;
    int count;
    int overvoltage_count;
    int undervoltage_count;
    int fault_count;
    int oor_count;
} ChannelStats;

// Records missing data sequences
typedef struct {
    uint32_t expected_seq;
    uint32_t found_seq;
    int gap_size;
    float timestamp;
} SequenceGap;

// Convert raw 12-bit ADC value to voltage
double convert_voltage(uint16_t raw_value);

// Compute statistics for all channels
void compute_channel_stats(const ADCSample *records, uint32_t count, ChannelStats stats[NUM_CHANNELS]);

// Check for missing sequence numbers
int check_sequence_integrity(const ADCSample *records, uint32_t count, SequenceGap *gaps, int max_gaps);

#endif /* ADC_H */
