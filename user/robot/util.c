// #include "util.h"

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

typedef struct rolling_buffer_f {
    uint16_t i;
    uint16_t size;
    float *buf;
} rolling_buffer_f;

void rolling_buffer_f_new(rolling_buffer_f *b, int16_t size) {
    b->i = 0;
    b->size = size;
    b->buf = malloc(sizeof(float) * size);
}

void rolling_buffer_f_add(rolling_buffer_f *b, float v) {
    b->buf[(b->i++) % b->size] = v;
}

float rolling_buffer_f_avg(rolling_buffer_f *b) {
    float avg = 0;
    for (int i = 0; i < b->size; i++) {
        avg += b->buf[i];
    }
    return avg / b->size;
}

float rolling_buffer_f_stddev(rolling_buffer_f *b) {
    float avg = rolling_buffer_f_avg(b);
    float dev = 0;
    for (int i = 0; i < b->size; i++) {
        dev += square(avg - b->buf[i]);
    }
    return sqrt(dev);
}