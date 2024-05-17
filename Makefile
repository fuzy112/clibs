# Copyright © 2024  Zhengyi Fu <i@fuzy.me>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

CONFIG_DEBUG := 1
CONFIG_WARN := 1

SHELL = /bin/sh

ifeq ($(CONFIG_GC), 1)
LDLIBS += -lgc
CFLAGS += -Dmalloc=GC_malloc -Dfree=GC_free -Drealloc=GC_realloc
endif

ENABLE_SANITIZER = -fsanitize=address,undefined -O1 -fno-omit-frame-pointer

ifeq ($(CONFIG_SANITIZER), 1)
CFLAGS += $(ENABLE_SANITIZER)
LDFLAGS += $(ENABLE_SANITIZER)
endif

ifeq ($(CONFIG_COVERAGE), 1)
CFLAGS += -fcoverage-mapping -fprofile-instr-generate
LDFLAGS += -fcoverage-mapping -fprofile-instr-generate
endif

ifndef CHK_SOURCES
CFLAGS += -MMD -MP
endif

ifeq ($(CONFIG_DEBUG), 1)
CFLAGS += -g3
endif

ifeq ($(CONFIG_WARN), 1)
CFLAGS += -Wall -Wextra -Werror
endif

OPTS ?=

CFLAGS += $(OPTS) -std=gnu89

LDFLAGS += $(OPTS)


EXE=

.PHONY: all

targets := avltree-test rbtree-test \
           avl2dot rb2dot genrnd list-test \
           xarray-test urlencode-test \
	   circbuf-test hashtable-test

all: $(targets:%=%$(EXE))

avltree-test$(EXE): avltree-test.o avltree.o

rbtree-test$(EXE): rbtree.o rbtree-test.o

avl2dot$(EXE): avltree.o tree2dot.o avltree2dot.o
	$(LINK.c) -o $@ $^ $(LDLIBS)

rb2dot$(EXE): rbtree.o tree2dot.o rbtree2dot.o
	$(LINK.c) -o $@ $^ $(LDLIBS)

list-test$(EXE): list-test.o

circbuf-test$(EXE): circbuf-test.o circbuf.o

xarray-test$(EXE): xarray-test.o xarray.o

genrnd$(EXE): genrnd.o

urlencode-test$(EXE): urlencode-test.o urlencode.o

hashtable-test$(EXE): hashtable-test.o

xarray-fuzzer: xarray.o
	$(LINK.c) -fsanitize=fuzzer -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	-$(RM) -- *.d *.o $(targets:%=%$(EXE))

*.o: Makefile

.PHONY: check-syntax

# special target used by Emacs' FlyMake plugin
check-syntax:
	$(CC) -S -Wall -Wextra -o /dev/null $(CPPFLAGS) $(CFLAGS) $(CHK_SOURCES)

-include *.d

