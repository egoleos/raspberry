CC = gcc
CFLAGS =  -std=c99 -I. -lbcm2835
DEPS = 
OBJ = M1Wire.o

%.o: %.c $(DEPS)
	$(CC) -c -Ofast $@ $< $(CFLAGS)

M1Wire: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)
