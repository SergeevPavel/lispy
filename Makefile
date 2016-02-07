
all: release

debug:
	cc -std=c99 -Wall -ledit -g -O0 -lm prompt.c mpc.c -o prompt

release:
	cc -std=c99 -Wall -ledit -O2 -lm prompt.c mpc.c -o prompt
clean:
	rm prompt

.PHONY:
	clean
	all
