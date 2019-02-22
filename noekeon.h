/* The following code was modified from NoekeonDirectRef.c
 * Written by : Michael Peeters */

#include <stdint.h>

#define u8  uint8_t
#define u32 uint32_t

#define T8(x)   ((x) & 0xff)
#define T32(x)  ((x) & 0xffffffff)

#define U8TO32_BIG(c)  (((u32)T8(*(c)) << 24) | ((u32)T8(*((c) + 1)) << 16) |\
                       ((u32)T8(*((c) + 2)) << 8) | ((u32)T8(*((c) + 3))))

#define U32TO8_BIG(c, v)    do { \
		u32 x = (v); \
		u8 *d = (c); \
		d[0] = T8(x >> 24); \
		d[1] = T8(x >> 16); \
		d[2] = T8(x >> 8); \
		d[3] = T8(x); \
	} while (0)

#define ROTL32(v, n)   (T32((v) << (n)) | ((v) >> (32 - (n))))

void Theta1 (u32 * const a);
void Theta2 (u32 * const a);
void get_const(u32 rc[4], int round);
void Pi1(u32 * const a);
void Pi2(u32 * const a);
void Gamma(u32 * const a);

// -------------------------------------------------- interface for auto.c

#define STATE_SIZE 128
#define CONST_NUM  16

void noekeon_init() {}

void noekeon_const(void *data, int i) {
  get_const((u32*) data,i);
}

void noekeon(void *data) {
  u32 *s = (u32*) data;
  Theta2(s);
  Pi1(s);
  Gamma(s);
  Pi2(s);
  Theta1(s);
}

// -------------------------------------------------- implementation

/*==================================================================================*/
/* Null Vector
/*----------------------------------------------------------------------------------*/
u32 NullVector[4] = {0,0,0,0};


void Theta1 (u32 * const a) {
  u32 tmp;
  tmp  = a[0]^a[2]; 
  tmp ^= ROTL32(tmp,8)^ROTL32(tmp,24); 
  a[1]^= tmp; 
  a[3]^= tmp; 
}

void Theta2 (u32 * const a) {
  u32 tmp;
  tmp  = a[1]^a[3]; 
  tmp ^= ROTL32(tmp,8)^ROTL32(tmp,24); 
  a[0]^= tmp; 
  a[2]^= tmp; 
}

void get_const(u32 rc[4], int round) {
  u8 RC = 0x80;
  int i;
  for (i=0;i<round;i++)
    if (RC&0x80)
       RC =(RC<<1) ^ 0x1B;
    else
      RC <<= 1;
  rc[0] = RC;
  rc[1] = rc[2] = rc[3] = 0;
  Theta1(rc);
}


/*==================================================================================*/
void Pi1(u32 * const a)
/*----------------------------------------------------------------------------------*/
/* DISPERSION - Rotations Pi1
/*==================================================================================*/
{ a[1] = ROTL32 (a[1], 1); 
  a[2] = ROTL32 (a[2], 5); 
  a[3] = ROTL32 (a[3], 2); 
}  /* Pi1 */


/*==================================================================================*/
void Pi2(u32 * const a)
/*----------------------------------------------------------------------------------*/
/* DISPERSION - Rotations Pi2
/*==================================================================================*/
{ a[1] = ROTL32 (a[1], 31);
  a[2] = ROTL32 (a[2], 27); 
  a[3] = ROTL32 (a[3], 30); 
}  /* Pi2 */


/*==================================================================================*/
void Gamma(u32 * const a)
/*----------------------------------------------------------------------------------*/
/* NONLINEAR - gamma, involution
/*----------------------------------------------------------------------------------*/
/* Input of i_th s-box = (i3)(i2)(i1)(i0), with (i3) = i_th bit of a[3]
 *                                              (i2) = i_th bit of a[2]
 *                                              (i1) = i_th bit of a[1]
 *                                              (i0) = i_th bit of a[0]
 *
 * gamma = NLIN o LIN o NLIN : (i3)(i2)(i1)(i0) --> (o3)(o2)(o1)(o0)
 *
 * NLIN ((i3) = (o3) = (i3)                     NLIN is an involution
 *       (i2)   (o2)   (i2)                      i.e. evaluation order of i1 & i0
 *       (i1)   (o1)   (i1+(~i3.~i2))                 can be swapped
 *       (i0))  (o0)   (i0+(i2.i1))
 * 
 *  LIN ((i3) = (o3) = (0.i3+0.i2+0.i1+  i0)    LIN is an involution
 *       (i2)   (o2)   (  i3+  i2+  i1+  i0)    
 *       (i1)   (o1)   (0.i3+0.i2+  i1+0.i0)    
 *       (i0))  (o0)   (  i3+0.i2+0.i1+0.i0)    
 *
/*==================================================================================*/
{ u32 tmp;

  /* first non-linear step in gamma */
  a[1] ^= ~a[3] & ~a[2];
  a[0] ^=   a[2] & a[1];

  /* linear step in gamma */
  tmp   = a[3];
  a[3]  = a[0];
  a[0]  = tmp;
  a[2] ^= a[0]^a[1]^a[3];

  /* last non-linear step in gamma */
  a[1] ^= ~a[3] & ~a[2];
  a[0] ^=   a[2] & a[1];
} /* Gamma */


/*==================================================================================*/
void Round (u32 const * const k,u32 * const a)
/*----------------------------------------------------------------------------------*/
/* The round function, common to both encryption and decryption
/* - Round constants is added to the rightmost byte of the leftmost 32-bit word (=a0)
/*==================================================================================*/
{ 
  Theta1(a); 
  a[0] ^= k[0]; a[1] ^= k[1]; a[2] ^= k[2]; a[3] ^= k[3]; 
  Theta2(a);
  Pi1(a); 
  Gamma(a); 
  Pi2(a); 
}  /* Round */

/*==================================================================================*/
void RCShiftRegFwd (u8 * const RC)
/*----------------------------------------------------------------------------------*/
/* The shift register that computes round constants - Forward Shift
/*==================================================================================*/
{ 

  if ((*RC)&0x80) (*RC)=((*RC)<<1) ^ 0x1B; else (*RC)<<=1;
  
} /* RCShiftRegFwd */


/*==================================================================================*/
void CommonLoop (u32 const * const k,u32 * const a, u8 RC1)
/*----------------------------------------------------------------------------------*/
/* loop - several round functions, ended by theta
/*==================================================================================*/
{ 
  unsigned i;
  u32 rc[4];
  for(i=0 ; i<16 ; i++) {
    Theta1(a); 
    get_const(rc,i);
    a[0] ^= k[0]^rc[0]; a[1] ^= k[1]^rc[1]; a[2] ^= k[2]^rc[2]; a[3] ^= k[3]^rc[3]; 
    Theta2(a);
    Pi1(a); 
    Gamma(a); 
    Pi2(a); 
  }
  Theta1(a); 
  get_const(rc,16);
  a[0] ^= k[0]^rc[0]; a[1] ^= k[1]^rc[1]; a[2] ^= k[2]^rc[2]; a[3] ^= k[3]^rc[3]; 
  Theta2(a);
} /* CommonLoop */


/*==================================================================================*/
void NESSIEencrypt(u32 k[4], 
                   const unsigned char * const plaintext,
                   unsigned char * const ciphertext)
/*==================================================================================*/
{  u32 state[4];
  
  state[0]=U8TO32_BIG(plaintext   );
  state[1]=U8TO32_BIG(plaintext+4 );
  state[2]=U8TO32_BIG(plaintext+8 );
  state[3]=U8TO32_BIG(plaintext+12);

  CommonLoop (k,state,0x80);
  
  U32TO8_BIG(ciphertext   , state[0]);
  U32TO8_BIG(ciphertext+4 , state[1]);
  U32TO8_BIG(ciphertext+8 , state[2]);
  U32TO8_BIG(ciphertext+12, state[3]);
} /* NESSIEencrypt */

void NESSIEkeysetup(const unsigned char * const key, u32 * k) { 
  k[0]=U8TO32_BIG(key   );
  k[1]=U8TO32_BIG(key+4 );
  k[2]=U8TO32_BIG(key+8 );
  k[3]=U8TO32_BIG(key+12);
}


