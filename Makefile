#Makefile for Assignment 1

SRC    = myshell.c
LEX    = lex.c
LGEN   = $(LEX:.c=.yy.c)
OBJ    = $(SRC:.c=.o) $(LGEN:.c=.o)
MKFILE = Makefile
README = README
JUNK   = $(OBJ) $(LGEN)
EXE    = myshell

GCC    = gcc
FLAGS  = -Wall -Wextra
LFLAG  = -lfl
FLEX   = flex

.SECONDARY:

all: $(EXE)

$(EXE): $(OBJ)
	$(GCC) -o $@ $^ $(LFLAG)

%.yy.o:%.yy.c
	$(GCC) -c $^

%.yy.c:%.c
	$(FLEX) $^

%.o:%.c
	$(GCC) $(FLAGS) -c $^

clean:
	rm $(JUNK)

spotless: clean
	rm $(EXE)
