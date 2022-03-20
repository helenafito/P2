#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "pav_analysis.h"
#include "vad.h"

const float FRAME_TIME = 10.0F; /* in ms. */

/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT","MV","MS",
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

Features compute_features(const float *x, int N) {
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
  feat.zcr = compute_zcr(x, N, 16000);
  feat.p = compute_power(x, N);
  feat.am = compute_am(x, N);


  return feat;
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate, float alpha1) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  vad_data->alpha1 = alpha1;
  vad_data->potsil = -1e12;
  vad_data->Ninitmax = 9 ;
  vad_data->ninit = 0;
  vad_data->counter=0;
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */
  VAD_STATE state = /*vad_data->state*/ST_SILENCE;

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
  
  
  Features f = compute_features(x, vad_data->frame_length);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state) {
  case ST_INIT://contador que se queda en el init
    //vad_data->k1 = f.p + vad_data->alpha1;
    if ( vad_data->ninit < vad_data->Ninitmax){

     //vad_data->potsil+= pow(10,(f.p/10));
      vad_data->potsil = fmax(f.p,vad_data->potsil + 0.1); //+ vad_data->potsil;
      vad_data->ninit++;
    }

    else{
      vad_data->k1 =vad_data->potsil+vad_data->alpha1 + 0.1;
      vad_data->k2 = vad_data->potsil + 7.3;
      vad_data->state = ST_SILENCE;
       }
    
    break;

  case ST_SILENCE:
    if (f.p > vad_data->k1 - 0.4){
      vad_data->state = ST_MV;
    }
    break;

  case ST_VOICE:
    if (f.p < vad_data->k1 + 0.1 ){
      vad_data->state = ST_MS;
    }
    break;

  case ST_MS:
  vad_data->counter++;
   if (vad_data->counter== 4){
     vad_data->state = ST_SILENCE;
     vad_data->counter=0;
   }
   if(f.p > vad_data->k1 && f.zcr){
     vad_data->state = ST_VOICE;
     vad_data->counter=0;
   }
   if(f.am > 1.9e-4 ){
    vad_data->state = ST_MV;
    vad_data->counter=0;
   }

   if(f.p > vad_data->k1 && f.p < vad_data->k1 + 0.8 ){
      vad_data->state = ST_VOICE;
       vad_data->counter=0;
    }
  
  break;

  case ST_MV:
  vad_data->counter++;
    if(f.p > vad_data->k2 - 0.2){
      vad_data->state = ST_VOICE;
      vad_data->counter=0;
    }
    if(f.am<1e-4 ){
    vad_data->state = ST_SILENCE;
    vad_data->counter=0;
   }
    else if(vad_data->counter==9){
      vad_data->state = ST_SILENCE;
      vad_data->counter=0;
    }
    
  break;

  case ST_UNDEF:
    break;
  }

  if(vad_data->state == ST_INIT){
    return ST_SILENCE;
  }
  if (vad_data->state == ST_SILENCE ||
      vad_data->state == ST_VOICE)
    return vad_data->state;
  else
    return ST_UNDEF;
    
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}
