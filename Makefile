DATE=    07-Mar-2013
#WAS=    24-Aug-1998
VERSION= 1.09.2

HDR=     Calc.h Keys.h Main.h Memory.h
OBJ=	 ascii.o binary.o calc.o keys.o main.o memory.o req.o
SRC=	 ascii.c binary.c calc.c keys.c main.c memory.c req.c
MISC=    CHANGES COPYING ycalc.1 Makefile

LIB=	 -L/usr/X11/lib/ -lm -lX11 -lc

# optimizing

#CCFLAGS= +O3 -Aa  # hpux
#CCFLAGS= -O6      # linux

# debugging

#CCFLAGS= -g -Aa   # hpux
CCFLAGS= -g        # linux

ycalc:	${OBJ}
	cc  ${OBJ} $(LIB) -o ycalc

.c.o:
	cc -c ${CCFLAGS} -DVERSION='"${VERSION}"' -DDATE='"${DATE}"' $<

ascii.o:  Keys.h Main.h
binary.o: Keys.h Main.h
calc:     Calc.h Keys.h Main.h Memory.h
keys.o:   Keys.h Main.h
main.o:   Calc.h Keys.h Main.h Memory.h
memory.o: Calc.h Keys.h Main.h Memory.h
req.o:    Keys.h Main.h

clean:
	rm -f ${OBJ} ycalc

dist:
	rm -rf ycalc-${VERSION} ycalc-${VERSION}.tar.gz
	mkdir ycalc-${VERSION}
	cp ${HDR} ${SRC} ${MISC} ycalc-${VERSION}
	tar cfz ycalc-${VERSION}.tar.gz ycalc-${VERSION}
	rm -rf ycalc-${VERSION}
