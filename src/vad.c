#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "vad.h"
#include "pav_analysis.h"

const float FRAME_TIME = 10.0F; /* in ms. */

/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT", "MV","MS"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;

/* 
 * TODO: Delete and use your own features!
 */

Features compute_features(const float *x, int N, float fm) {
  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */
  /* 
   * DELETE and include a call to your own functions
   *
   * For the moment, compute random value between 0 and 1 
   */
  Features feat;
  feat.p = compute_power(x,N);
  feat.am = compute_am(x,N);
  feat.zcr = compute_zcr(x,N,fm);
  //feat.zcr = feat.p = feat.am = (float) rand()/RAND_MAX;
  return feat;
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  vad_data->frame_cnt = 0;
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */
  VAD_STATE state = vad_data->state;

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * TODO: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x) {

  /* 
   * TODO: You can change this, using your own features,
   * program finite state automaton, define conditions, etc.
   */

  Features f = compute_features(x, vad_data->frame_length, vad_data->sampling_rate);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state) {
  case ST_INIT:

    /*
    vad_data->state = ST_SILENCE;
    vad_data->zcr_0 = 7000;
    vad_data->min_voice = 6;
    vad_data->min_silence = 6;
    vad_data->maybe_cnt = 0;
    vad_data->alfa1 = 2;//2
    vad_data->alfa2 = 5;//5
    vad_data->k0 = f.p;
    vad_data->k1 = vad_data->k0 + vad_data->alfa1;
    vad_data->k2 = vad_data->k1 + vad_data->alfa2;
    */

    if (vad_data->frame_cnt < 10) {
      vad_data->k0 += pow(10,f.p/10);
      vad_data->frame_cnt++;
      if(vad_data->frame_cnt == 10){
        vad_data->k0 = 10*log10(vad_data->k0/vad_data->frame_cnt);
        vad_data->alfa1 = 1.7;
        vad_data->alfa2 = 4.5;
        vad_data->k1 = vad_data->k0 + vad_data->alfa1;
        vad_data->k2 = vad_data->k1 + vad_data->alfa2;
        vad_data->min_voice = 7; 
        vad_data->min_silence = 8;
        vad_data->maybe_cnt = 0;
        vad_data->zcr_0 = 3000;
        vad_data->state = ST_SILENCE;
      }
    }

    break;

  case ST_SILENCE:
    if (f.p > vad_data->k1 || f.zcr > vad_data->zcr_0) 
      vad_data->state = ST_MAYBE_VOICE;
    break;

  case ST_VOICE:
    if (f.p < vad_data->k2 && f.zcr < vad_data->zcr_0) 
      vad_data->state = ST_MAYBE_SILENCE;
    break;

  case ST_MAYBE_VOICE:
    vad_data->maybe_cnt++;
    if ((f.p > vad_data->k2 || f.zcr > vad_data->zcr_0) && vad_data->maybe_cnt >= vad_data->min_voice) { 
      vad_data->state = ST_VOICE;
      vad_data->maybe_cnt = 0;
    } else if (f.p < vad_data->k1 && f.zcr < vad_data->zcr_0) { 
      vad_data->state = ST_SILENCE;
      vad_data->maybe_cnt = 0;
    } 
    break;

  case ST_MAYBE_SILENCE:
    vad_data->maybe_cnt++;
    if (f.p < vad_data->k1 && f.zcr < vad_data->zcr_0 && vad_data->maybe_cnt >= vad_data->min_silence) { 
      vad_data->state = ST_SILENCE;
      vad_data->maybe_cnt = 0;
    } else if (f.p > vad_data->k2 || f.zcr > vad_data->zcr_0) { 
      vad_data->state = ST_VOICE;
      vad_data->maybe_cnt = 0;
    }
    break;

  case ST_UNDEF:
    break;
  }

  if (vad_data->state == ST_SILENCE ||
      vad_data->state == ST_VOICE ||
      vad_data->state == ST_MAYBE_VOICE ||
      vad_data->state == ST_MAYBE_SILENCE )
    return vad_data->state;
  else
    return ST_UNDEF;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}
