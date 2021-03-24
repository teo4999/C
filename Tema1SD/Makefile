CC=gcc
CFLAGS=-Wall -Wextra -g
EXE=atm

build: $(EXE)


$(EXE): $(EXE).c
	$(CC) $(CFLAGS) $^ -o $@

run:
	./$(EXE)

clean:
	rm -f $(EXE)