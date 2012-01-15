#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

typedef struct rolling_buffer_f {
    uint16_t i;
    uint16_t size;
    float *buf;
} rolling_buffer_f;

void rolling_buffer_f_new(rolling_buffer_f *b, int16_t size);

void rolling_buffer_f_add(rolling_buffer_f *b, float v);

float rolling_buffer_f_avg(rolling_buffer_f *b);

float rolling_buffer_f_stddev(rolling_buffer_f *b);