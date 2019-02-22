#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

//number of cores
#define NTHREAD        4

//if zero, no display of number of tries
//otherwise, number of tries displayed every 2^DISPLAY_RATE tries (should be > 4)
#define DISPLAY_RATE   16

// -- end of parameters --

#define STRING(s)      #s
#define EVAL_STRING(s) STRING(s)
#define CAT(x,y)       x##y
#define EVAL_CAT(x,y)  CAT(x,y)

#include EVAL_STRING(TARGET.h)
#include "vector.h"

#define FUNC           TARGET
#define INIT           EVAL_CAT(TARGET,_init)
#define CONST          EVAL_CAT(TARGET,_const)
#define PRINT_STATE    EVAL_CAT(TARGET,_print_state)

pthread_mutex_t display_lock;
pthread_mutex_t count_lock;

time_t start;
u64 global_tries;

// -------------------------------------------------- auxiliary

int prepare_nucleon(table_t tab) {
  int i;
  int dim = 0;
  vector_t vec;
  table_zero(tab);
  for (i=0;i<CONST_NUM;i++) {
    CONST(vec,i);
    dim += table_add_vec(tab, vec);
  }
  return dim;
}

void print_progress() {
  pthread_mutex_lock(&display_lock);
  printf("\rtries = %8llx x 2^%d",(long long unsigned) global_tries>>DISPLAY_RATE,DISPLAY_RATE);
  fflush(stdout);
  pthread_mutex_unlock(&display_lock);
}

// -------------------------------------------------- try offset

static int try_offset(vector_t offset, int dim, table_t tab) {
  u64 tweak = 0;
  vector_t coef, vec, image;
  unsigned fail_count = 0;

  vector_copy(coef,offset);
  vector_copy(image,offset);
  FUNC(image);

  while (fail_count<50) {
    vector_randomize(coef,tweak--);
    table_mult_vector(vec,tab,coef);
    vector_xor(vec,offset);
    FUNC(vec);
    vector_xor(vec,image);
    if (table_add_vec(tab, vec)) {
      fail_count = 0;
      dim++;
    } else
      fail_count++;
    if (dim==STATE_SIZE)
      return 0;
  }
  pthread_mutex_lock(&display_lock);
  time_t t = time(NULL) - start;
  printf("\r Found space of dim %3d! after %4u'%2u  offset: ", dim, (unsigned) t/60, (unsigned) t%60);
  vector_print(offset);
  puts("");
  pthread_mutex_unlock(&display_lock);
#if DISPLAY_RATE
  print_progress();
#endif
  return 1;
}

// -------------------------------------------------- worker threads

typedef struct {
  u64 seed;
} info_t;

void *work(void *info) {
  vector_t seed;
  u64 tweak = 0;
  table_t *tab_nucleon, *tab;
  if (!(tab = malloc(sizeof(table_t))) || !(tab_nucleon = malloc(sizeof(table_t)))) {
    fprintf(stderr,"%s: could not allocate memory for tables (%lu bytes).\n",
        __func__,(long unsigned) sizeof(table_t));
    exit(1);
  }
  int dim = prepare_nucleon(*tab_nucleon);
  u64 local_tries = 0;

  vector_zero(seed);
  vector_randomize(seed,((info_t*) info)->seed);

  while (1) {
    vector_randomize(seed,tweak++);
    //*seed |= 0xff00ff00ff00ff00;//robin hack
    table_copy(*tab,*tab_nucleon);
#if DISPLAY_RATE
    local_tries++;
    if (local_tries == 1UL<<(DISPLAY_RATE-4)) {
      pthread_mutex_lock(&count_lock);
      global_tries += local_tries;
      if(!(global_tries&((1UL<<(DISPLAY_RATE))-1)))
    print_progress();
      pthread_mutex_unlock(&count_lock);
      local_tries = 0;
    }
#endif
    if (try_offset(seed, dim, *tab) == -1)
      break;
  }

  free(tab_nucleon);
  free(tab);
  return NULL;
}

void launch(u64 global_seed) {
  info_t info[NTHREAD];
  pthread_t tid[NTHREAD];
  int i;

  srandom(global_seed);
  for (i=0;i<NTHREAD;i++) {
    info[i].seed = 0xffff * global_seed + random();
    if ( pthread_create(&(tid[i]),NULL,work,(void*) &(info[i])) ) {
      fprintf(stderr,"%s: couldn't create the %d-th thread\n",__func__,i+1);
      exit(1);
    }
  }

  for (i=0;i<NTHREAD;i++)
    pthread_join(tid[i], NULL);
}

// -------------------------------------------------- functions for tests

int try_one_offset(vector_t offset) {
  table_t tab;
  int dim = prepare_nucleon(tab);

  return try_offset(offset, dim, tab);
}

// -------------------------------------------------- main

int main(int argc, char **argv) {
  if (pthread_mutex_init(&display_lock, NULL) || pthread_mutex_init(&count_lock, NULL)) {
    fprintf(stderr,"%s: mutex initialization failed.\n",__func__);
    exit(1);
  }

  INIT();

  /* vector_t data; */
  /* vector_zero(data); */
  /* vector_randomize(data,0); */
  /* PRINT_STATE(data); */
  /* vector_print(data); */
  /* puts(""); */
  /* FUNC(data); */
  /* PRINT_STATE(data); */
  /* vector_print(data); */
  /* puts(""); */
  /* CONST(data,0); */
  /* PRINT_STATE(data); */
  /* vector_print(data); */
  /* puts(""); */
  /* return 1; */

  u64 global_seed = 0;
  if (argc > 1)
    global_seed = atoi(argv[1]);
  printf("Target = " EVAL_STRING(TARGET) ", seed = %lu\n",(long unsigned) global_seed);
  table_t tab;
  printf("nucleon dim = %d\n", prepare_nucleon(tab));

  start = time(NULL);
  global_tries = 0;

  vector_t zero;
  vector_zero(zero);
  try_one_offset(zero);

  launch(global_seed);

  pthread_mutex_destroy(&display_lock);
  pthread_mutex_destroy(&count_lock);
  return 0;
}
