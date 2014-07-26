# catwm - cute and tile window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = catwm.c
OBJ = ${SRC:.c=.o}

all: options catwm

options:
	@echo catwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

dwm: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f catwm ${OBJ}

dist: clean
	@echo creating dist tarball
	@mkdir -p catwm-${VERSION}
	@cp -R LICENCE_DWM Makefile README.md config.def.h config.h config.mk \
		catwm.1 ${SRC} catwm-${VERSION}
	@tar -cf catwm-${VERSION}.tar catwm-${VERSION}
	@gzip catwm-${VERSION}.tar
	@rm -rf catwm-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f catwm ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/catwm
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < catwm.1 > ${DESTDIR}${MANPREFIX}/man1/catwm.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/catwm.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/catwm
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/catwm.1

.PHONY: all options clean dist install uninstall
