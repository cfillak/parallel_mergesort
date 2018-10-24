CC = gcc
INC = -I.
FLAGS = -W -Wall -g
LINKOPTS = -pthread

all: msort

msort: msort.c
	$(CC) $(INC) $(FLAGS) $(LINKOPTS) msort.c -o msort

clean:
	rm -rf msort
