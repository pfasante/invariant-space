Invariant Subspace Cryptanalysis
================================

This code is copied from
[invariant-space.gforge.inria.fr](http://invariant-space.gforge.inria.fr/),
the corresponding paper is [A Generic Approach to Invariant Subspace Attacks:
Cryptanalysis of Robin, iSCREAM and Zorro](https://eprint.iacr.org/2015/068),
by Gregor Leander and Brice Minaud and Sondre Rønjom.



The 'auto' program implements an automatic search for invariant subspaces.

The program itself is generic and independent of any primitive. Primitives
come as a form of plugin to the main program, in separate header files.

The main program, auto.c, as well as the Zorro, Robin and Fantomas plugins,
are in the public domain.

Acknowledgment: the Robin and Fantomas plugins are directly adapted from
reference implementations provided by the LS-design authors.


How to try a built-in primitive
-------------------------------

Type in a terminal:
> make auto TARGET=zorro
to create the 'auto' binary for target cipher zorro.

How to plug in a new primitive
------------------------------

Say the new primitive is called mysterion.

Then a file called mysterion.h should be put in the same folder as auto.c

This file should contain the following information:

  #define STATE_SIZE [size of the state in bits]
  #define CONST_NUM  [number of round constants]

  void mysterion_init();
This function will be called once at startup. This should contain any necessary
setup before the cipher can be used, such as precomputing a table. For most
implementations, there is no such setup, so this function can be empty.

  void mysterion_const(void *data, int i);
This should write the i-th STATE_SIZE-bit round constant at the memory location
'data' points to.

  void mysterion(void *data);
This should perform the mysterion round function on the STATE_SIZE-bit data. The
bit ordering is unimportant as long as it is consistent with mysterion_const().

That is all. The 'auto' binary can be created for mysterion with:
> make auto TARGET=mysterion

Of course if the new primitive relies on particular object files being compiled,
the Makefile should be modified accordingly.

Other parameters
----------------

NTHREAD in auto.c should be set to the number of available cores.

The 'auto' program accepts a single integer parameter. This is used as the random
seed (zero by default). So './auto 1' and './auto 2' run independent tests.

Warning: for state sizes that are not multiple of 64 bits, little endian ordering
is assumed.
