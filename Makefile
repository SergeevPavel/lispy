
all: release

debug:
	cc -std=c99 -Wall -ledit -g -O0 -lm prompt.c mpc.c -o prompt

release:
	cc -std=c99 -Wall -ledit -O2 -lm prompt.c mpc.c -o prompt

profile:
	cc -std=c99 -Wall -ledit -pg -O2 -lm prompt.c mpc.c -o prompt

highlighting:
	cp lispy.vim ~/.vim/syntax/lispy.vim

clean:
	rm prompt
	rm gmon.out

.PHONY:
	clean
	all
	debug
	release
	profile
	highlighting
