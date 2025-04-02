FILES:=$(wildcard *.c)
FLAGS:=-Wall -Wextra -lSDL2 -lm -g

all:
	gcc -o game $(FILES) $(FLAGS)
