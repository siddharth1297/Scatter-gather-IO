SRCC=$(wildcard *.c)
CTARGETS=$(patsubst %.c,%,$(SRCC))

SRCCPP=$(wildcard *.cpp)
CPPTARGETS=$(patsubst %.cpp,%,$(SRCCPP))

default: $(CTARGETS)

OPT=-O0 -ggdb
#OPT=-O3

%: %.c
	gcc -std=c11 -Werror $(OPT) \
	        -o $*.out $<

default: $(CPPTARGETS)

%: %.cpp
	g++ -std=c++11 -Werror $(OPT) \
                -o $*.out $<

format:
	clang-format -i *.c *.h

clean:
	rm *.out
