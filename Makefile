#Makefile for Assignment 1

SRC    = myshell.c
LEX    = lex.c
LGEN   = $(LEX:.c=.yy.c)
OBJ    = $(SRC:.c=.o) $(LGEN:.c=.o)
MKFILE = Makefile
README = README
DESGN  = DESIGN
JUNK   = $(OBJ) $(LGEN)
ALL    = $(SRC) $(LEX) $(DESGN) $(MKFILE)
EXE    = myshell

GCC    = gcc
FLAGS  = -Wall -Wextra
LFLAG  = -lfl
FLEX   = flex

.SECONDARY:

all: $(EXE)

again: spotless all

$(EXE): $(OBJ)
	$(GCC) -o $@ $^ $(LFLAG)

%.yy.o:%.yy.c
	$(GCC) -c $^

%.yy.c:%.c
	$(FLEX) $^

%.o:%.c
	$(GCC) $(FLAGS) -c $^

tar:
	tar -cf $(EXE).tar $(ALL) 

clean:
	rm $(JUNK)

spotless: clean
	rm $(EXE)
