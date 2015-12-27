
all:
	cc -std=c99 -Wall -ledit prompt.c -o prompt

clean:
	rm prompt

.PHONY:
	clean
	all
