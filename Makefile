.POSIX:

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man

CFLAGS := $(CFLAGS) -DEXTENDED -I.
LIBS = -lm

OBJS = eval.o expr.o look.o main.o misc.o gnum4.o trace.o tokenizer.o parser.o ohash.o reallocarray.o strlcpy.o strtonum.o

all: om4

om4: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

parser.c parser.h: parser.y
	yacc -d parser.y && mv y.tab.c parser.c && mv y.tab.h parser.h

tokenizer.o: parser.h

install:
	install -d $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(MANDIR)/man1
	install -c -s -m 755 $(PROG) $(DESTDIR)$(BINDIR)
	install -c -m 644 m4.1 $(DESTDIR)$(MANDIR)/man1/$(PROG).1

test:
	@echo "No tests"

clean:
	rm -f $(PROG) $(OBJS) parser.c parser.h

distclean: clean
	rm -f Makefile config.h
