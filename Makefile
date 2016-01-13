
all:
	cc -std=c99 -Wall -ledit -lm prompt.c mpc.c -o prompt

clean:
	rm prompt

.PHONY:
	clean
	all
