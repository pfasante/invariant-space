#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../keccak.h"
#include "Keccak-f.h"

#define u64 uint64_t

#define LANE_LENGTH ((STATE_SIZE)/25)

#define N (((STATE_SIZE-1)/64)+1)

#if LANE_LENGTH <= 4
#define PRINT_SIZE "1"
#elif LANE_LENGTH == 8
#define PRINT_SIZE "2"
#elif LANE_LENGTH == 16
#define PRINT_SIZE "4"
#elif LANE_LENGTH == 32
#define PRINT_SIZE "8"
#elif LANE_LENGTH == 64
#define PRINT_SIZE "16"
#endif

using namespace std;

void data_from_LaneValue(void *data_void, vector<LaneValue> A) {
  u64 *data = (u64*) data_void;
  for (unsigned i=0;i<N;i++)
    data[i] = 0;
#if LANE_LENGTH == 1
  for (unsigned i=0;i<25;i++)
    *data |= A[i]<<(/*64-25+*/i);
#elif LANE_LENGTH == 2
  for (unsigned i=0;i<25;i++)
    *data |= A[i]<<(/*64-50+*/2*i);
#elif LANE_LENGTH == 4
  for (unsigned i=0;i<16;i++)
    data[0] |= A[i]<<(4*i);
  for (unsigned i=0;i<9;i++)
    data[1] |= A[16+i]<<(/*64-36+*/4*i);
#elif LANE_LENGTH >= 8
  uint8_t *data8 = (uint8_t *) data;
  for (unsigned i=0;i<25;i++)
    memcpy(data8+i*LANE_LENGTH/8,&(A[i]),LANE_LENGTH/8);
#else
#error invalid Keccak state size
#endif
}

void LaneValue_from_data(vector<LaneValue> *A, void *data_void) {
  u64 *data = (u64*) data_void;
#if LANE_LENGTH == 1
  for (unsigned i=0;i<25;i++)
    (*A)[i] = ((*data)>>(/*64-25+*/i))&1;
#elif LANE_LENGTH == 2
  for (unsigned i=0;i<25;i++)
    (*A)[i] = ((*data)>>(/*64-50+*/2*i))&3;
#elif LANE_LENGTH == 4
  for (unsigned i=0;i<16;i++)
    (*A)[i] = (data[0]>>(4*i))&0xf;
  for (unsigned i=0;i<9;i++)
    (*A)[16+i] = (data[1]>>(/*64-36+*/4*i))&0xf;
#elif LANE_LENGTH >= 8
  uint8_t *data8 = (uint8_t *) data;
  for (unsigned i=0;i<25;i++)
    memcpy(&((*A)[i]),data8+i*LANE_LENGTH/8,LANE_LENGTH/8);
#endif
}

// -------------------------------------------------- interface for auto.c

extern "C" {

void keccak_print_state(void *data) {
  vector<LaneValue> A(25);
  LaneValue_from_data(&A,data);
  for (unsigned i=0;i<5;i++) {
    for (unsigned j=0;j<5;j++)
      printf("%" PRINT_SIZE "lx ",(long unsigned) A[5*i+j]);
    puts("");
  }
}

void keccak_init() {}

void keccak_const(void *data, int i) {
  try {
    KeccakF keccakF(STATE_SIZE);
    vector<LaneValue> A(25);
    for (unsigned i=0;i<25;i++)
      A[i] = 0;
    keccakF.iota(A,0);
    data_from_LaneValue(data,A);
  }
  catch(KeccakException e) {
    cout << e.reason << endl;
    exit(EXIT_FAILURE);
  }  
}

void keccak(void *data) {
  try {
    KeccakF keccakF(STATE_SIZE);
    vector<LaneValue> A(25);

    LaneValue_from_data(&A, data);
    keccakF.theta(A);
    keccakF.rho(A);
    keccakF.pi(A);
    keccakF.chi(A);
    data_from_LaneValue(data,A);
  }
  catch(KeccakException e) {
    cout << e.reason << endl;
    exit(EXIT_FAILURE);
  }
}

}
