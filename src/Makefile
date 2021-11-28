CC=gcc
C-FLAGS=-O3 -Wall -Werror

all: lc3_vm

lc3_vm: main.c lc3_vm.c win_tools.c
	${CC} ${C-FLAGS} $^ -o $@

.PHONY:
clean:
	rm -f lc3_vm.exe
