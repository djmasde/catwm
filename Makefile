CFLAGS+= -Wall -std=c99
LDADD+= -lX11 
LDFLAGS=
EXEC=catwm

PREFIX?= /usr
BINDIR?= $(PREFIX)/bin

CC=cc

all: $(EXEC)

catwm: catwm.o
	$(CC) $(LDFLAGS) -Os -O2 $(CFLAGS) -o $@ $+ $(LDADD)

install: all
	install -Dm 755 catwm $(DESTDIR)$(BINDIR)/catwm
	strip $(BINDIR)/catwm
	chmod 755 $(BINDIR)/catwm
clean:
	rm -f catwm *.o
uninstall:
	rm $(BINDIR)/catwm
