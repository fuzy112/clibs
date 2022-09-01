OPTS ?=

CFLAGS += -MMD -MP -g -Wall -Werror $(OPTS) -std=gnu89

LDFLAGS += $(OPTS)

CC=gcc

EXE=

.PHONY: all

targets := avltree-test rbtree-test \
	avl2dot rb2dot genrnd list-test \
    splay2dot xarray-test urlencode-test \
	circbuf-test hashtable-test

all: $(targets:%=%$(EXE))

avltree-test$(EXE): avltree-test.o avltree.o

rbtree-test$(EXE): rbtree.o rbtree-test.o

avl2dot$(EXE): avltree.o tree2dot.o avltree2dot.o
	$(LINK.c) -o $@ $^

rb2dot$(EXE): rbtree.o tree2dot.o rbtree2dot.o
	$(LINK.c) -o $@ $^

splay2dot$(EXE): splay2dot.o splay.o tree2dot.o

list-test$(EXE): list-test.o

circbuf-test$(EXE): circbuf-test.o circbuf.o

xarray-test$(EXE): xarray-test.o xarray.o

genrnd$(EXE): genrnd.o

urlencode-test$(EXE): urlencode-test.o urlencode.o

hashtable-test$(EXE): hashtable-test.o

.PHONY: clean
clean:
	-$(RM) *.d *.o $(targets:%=%$(EXE))

*.o: Makefile

.PHONY: check-syntax

# special target used by Emacs' FlyMake plugin
check-syntax:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CHK_SOURCES)

-include *.d
