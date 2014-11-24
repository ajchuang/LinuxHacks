CC := gcc
CFLAGS := -Wall -Werror -D__DEBUG__
LDFLAGS :=

OBJECTS := w4118_sh.o util.o str_list.o

all: w4118_sh


w4118_sh: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f w4118_sh
	rm -f *.o
	rm log.txt

.PHONY: clean
