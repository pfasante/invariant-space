CC = gcc
CCPP = g++
CFLAGS = -O3

BIN = auto

all: $(BIN)

auto: auto.c vector.h FORCE
ifeq ($(TARGET),)
	$(eval TARGET=zorro)
endif
ifeq ($(TARGET),keccak)
	cd keccak && $(CCPP) $(CFLAGS) -c keccak.cpp transformations.cpp Keccak-f.cpp
	$(CC) $(CFLAGS) -o auto.o -c auto.c -DTARGET=$(TARGET) -Ikeccak
	$(CC) $(CFLAGS) -o auto auto.o keccak/*.o -lpthread -lc++
else
	$(CC) $(CFLAGS) -o $@ $@.c -pthread -DTARGET=$(TARGET)
endif

FORCE:

clean:
	rm -f *.o *~ keccak/*.o \#*\#

cleanse: clean
	rm -f $(BIN)
