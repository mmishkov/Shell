#Makefile Assignment 1 Cmps 111

SRC    = myshell.c
LEX    = lex.c
YYS    = $(LEX:.c=.yy.c)
CSRC   = $(SRC) $(YYS)
OBJ    = $(CSRC:.c=.o)
JUNK   = $(OBJ) $(YYS)
ALL    = $(SRC) $(LEX) $(MKFILE) $(README)
TAR    = asg1.tar
EXE    = myshell
MKFILE = Makefile
README = README

GCC    = gcc
CFLAGS = -Wall -Wextra
FLEX   = flex

#To keep intermediate files...
.SECONDARY:

all: $(EXE)

$(EXE): $(OBJ)
	$(GCC) -o $@ $^ -lfl

%.yy.o: %.yy.c
	$(GCC) $(FLAGS) -c $^

%.yy.c: %.c
	$(FLEX) $^

%.o: %.c
	$(GCC) $(FLAGS) -c $^

clean:
	rm $(JUNK)

spotless:
	rm $(EXE) $(JUNK)

tar:
	tar -cf $(TAR) $(ALL)
