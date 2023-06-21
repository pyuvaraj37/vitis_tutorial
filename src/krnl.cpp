#include "constants.h"

extern "C" {
    void krnl(data_t* a, data_t* b, data_t* c) {
        #pragma HLS INTERFACE m_axi port = a bundle = gmem0
        #pragma HLS INTERFACE m_axi port = b bundle = gmem1
        #pragma HLS INTERFACE m_axi port = c bundle = gmem2

        for (int i = 0; i < DATA_SIZE; i++) {
            #pragma HLS UNROLL //factor=2
            c[i] = a[i] + b[i];
        }
    
    }
}