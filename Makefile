FILES:=$(wildcard src/*.c)
FLAGS:=-Wall -Wextra -lGL -lglfw -lm -g

all:
	gcc -o hehe $(FILES) $(FLAGS)
