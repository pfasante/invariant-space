/* The implementation of Keccak is directly adapted from the reference implementation by its designers. The reference implementation comes with the following header.
KeccakTools

The Keccak sponge function, designed by Guido Bertoni, Joan Daemen,
Michaël Peeters and Gilles Van Assche. For more information, feedback or
questions, please refer to our website: http://keccak.noekeon.org/

Implementation by the designers,
hereby denoted as "the implementer".

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

#define STATE_SIZE 100
#define CONST_NUM  1

#ifdef __cplusplus
extern "C" {
#endif

void keccak_init();
void keccak_const(void *data, int i);
void keccak(void *data);
void keccak_print_state(void *data);

#ifdef __cplusplus
}
#endif
