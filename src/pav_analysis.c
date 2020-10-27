#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x, unsigned int N) {
    
    float aux = 0;

    for(int i=0; i < N; i++){
        aux = aux + pow(x[i],2);
    }
    
    aux = aux/N;
    float PdB = 10*log10(aux);

    return PdB;
}

float compute_am(const float *x, unsigned int N) {
    float aux = 0; 

	for(int i=0; i<N; i++){
	
		aux = aux + fabs(x[i]);
	}
	
	float A = aux/N;

	return A;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    int d = 0;
	for(int i=1; i<N; i++){
	
		if (x[i]*x[i-1] < 0){
			d++;
		}
		
	}

	float ZCR = (fm/2)*(1./(N-1))*d;

	return ZCR; 
}