#include <stdint.h>

#define BASE 64

#define uchar unsigned char

typedef uint64_t    u64;

#if BASE == 64
typedef u64  base_t;
#else
#error invalid base size
#endif

#define N (((STATE_SIZE-1)/BASE)+1)

typedef base_t    vector_t[N];
typedef vector_t  table_t[STATE_SIZE];

#define ROTL(x,b) (u64)( ((x) << (b)) | ( (x) >> (64 - (b))) )

// -------------------------------------------------- vector

static void vector_zero(vector_t r) {
  int i;
  for (i=0;i<N;i++)
    r[i] = 0;
}

static int vector_is_zero(vector_t x) {
  int i;
  for (i=0;i<N;i++)
    if (x[i])
      return 0;
  return 1;
}

static void vector_copy(vector_t r, vector_t x) {
  int i;
  for (i=0;i<N;i++)
    r[i] = x[i];
}

static void vector_xor(vector_t r, vector_t x) {
  int i;
  for (i=0;i<N;i++)
    r[i] ^= x[i];
}

static uchar vector_get_bit(vector_t vec, unsigned pos) {
  return (vec[pos/BASE]>>((/*BASE-1-*/pos)%BASE))&1;
}

static void vector_randomize(vector_t seed, u64 tweak) {
  // SipHash-like pseudo random function
  int i, j;

  u64 *p = (u64*) seed;
  for (i=2;i<((STATE_SIZE-1)/64)+1;i++)
    p[0] ^= p[i];

  u64 v0; memcpy(&v0,p,8);
  u64 v1 = 0x646f72616e646f6dULL ^ tweak;
  u64 v2 = 0x6c7967656e657261ULL;
  u64 v3 = 0x0123456789abcdefULL;
  if ((STATE_SIZE-1)/64)
    memcpy(&v3,p+1,8);

  for (i=0;i<((STATE_SIZE-1)/64)+1;i+=2) {
    for (j=0; j<3; j++) {
      v0 += v1; v1=ROTL(v1,13); v1 ^= v0; v0=ROTL(v0,32);
      v2 += v3; v3=ROTL(v3,16); v3 ^= v2;
      v0 += v3; v3=ROTL(v3,21); v3 ^= v0;
      v2 += v1; v1=ROTL(v1,17); v1 ^= v2; v2=ROTL(v2,32);
    }
    p[i] = v0 ^ v1;
    if (i+1 < ((STATE_SIZE-1)/64)+1)
      p[i+1] = v2 ^ v3;
  }
#if STATE_SIZE%64
  //the following line assumes little endian ordering
  p[((STATE_SIZE-1)/64)] &= 0xffffffffffffffffULL>>(64-STATE_SIZE%64);
#endif
}

void vector_print(vector_t vec) {
  int i;
  u64 *p = (u64*) vec;
  for (i=(STATE_SIZE-1)/64;i>=0;i--)
    printf("%016llx",(long long unsigned) p[i]);
}

// -------------------------------------------------- table

static void table_zero(table_t tab) {
  int i;
  for (i=0;i<STATE_SIZE;i++)
    vector_zero(tab[i]);
}

static void table_copy(table_t r, table_t t) {
  int i;
  for (i=0;i<STATE_SIZE;i++)
    vector_copy(r[i],t[i]);
}

static int table_add_vec(table_t tab, vector_t vec) {
  //if the vector is in the span of vectors already in the table, return 0
  //otherwise, add vector to the table and return 1
  int i, j, k;
  uchar f;
  for (i=0;i<(STATE_SIZE%64?N-1:N);i++) {
    for (j=0;j<BASE;j++)
      if ((vec[i]>>j)&1) {
	f = 0;
	for (k=i;k<N;k++)
	  if (f)
	    vec[k] ^= tab[BASE*i+j][k];
	  else
	    if (tab[BASE*i+j][k]) {
	      vec[k] ^= tab[BASE*i+j][k];
	      f = 1;
	    }
	if (!f) {
	  vector_copy(tab[BASE*i+j],vec);
	  return 1;
	}
      }
  }
#if STATE_SIZE%64
  int remainder = STATE_SIZE-(N-1)*BASE;
  for (j=0;j<remainder;j++)
    if ((vec[i]>>(/*64-remainder+*/j))&1) {
      f = 0;
      for (k=i;k<N;k++)
	if (f)
	  vec[k] ^= tab[BASE*i+j][k];
	else
	  if (tab[BASE*i+j][k]) {
	    vec[k] ^= tab[BASE*i+j][k];
	    f = 1;
	  }
      if (!f) {
	vector_copy(tab[BASE*i+j],vec);
	return 1;
      }
    }
#endif
  return 0;
}

static void table_mult_vector(vector_t vec, table_t tab, vector_t coef) {
  int i;
  vector_zero(vec);
  for (i=0;i<STATE_SIZE;i++)
    if (vector_get_bit(coef,i))
      vector_xor(vec,tab[i]);
}

void table_print(table_t tab) {
  int i;
  for (i=0;i<STATE_SIZE;i++)
    if (!vector_is_zero(tab[i])) {
      printf("%3d: ",i);
      vector_print(tab[i]);
      puts("");
    }
}
