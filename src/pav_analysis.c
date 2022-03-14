#include <math.h>
#include "pav_analysis.h"
#define SGN(x) ((x>0) ? 1 : ((x<0) ? -1 : 0))

float compute_nl(const float *x){
    float noise_power = 1.0e-12f;

    for ( int i=0; i < 9600; i++){
        if(x[i+1]>x[i]){
            noise_power=x[i+1];
        }
    }
    return 10*log10f(noise_power);
}

float compute_power(const float *x, unsigned int N) {
    float power = 1.0e-12f;

    for(int i=0; i < N; i++) {
        power += x[i]*x[i];
    }
    return 10*log10f(power/N);
    
}

float compute_am(const float *x, unsigned int N) {
    float a = 0.0f;
    for (int i = 0; i < N; i++){
        a += fabsf(x[i]);
    }
    return a/N;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    float zcr = 0.0f;
    const float scaling = fm / (2*(N-1));
    for(int i = 1; i < N; i++){
        if(SGN(x[i]) != SGN(x[i-1])){
            zcr++;
        }
    }
    return scaling*zcr;
}
