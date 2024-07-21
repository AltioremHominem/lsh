CARGS= -Wall -Wextra

all:
	gcc -o nimsh src/main.c $(CARGS)