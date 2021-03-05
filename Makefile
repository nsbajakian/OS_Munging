# compiler flags:
# -g	adds debugging information
# -Wall turns on most, but not all compiler warnings
CC			= gcc
CFLAGS 		= -Wall -g -m64
LDFLAGS		=
OBJFILES	= q1.o m8.o m16.o m32.o m64.o master.o

SOURCES = q1.c m8.c m16.c m32.c m64.c master.o

.PHONY: all
all: m8 m16 m32 m64 master

m8: m8.c
	$(CC) $(CFLAGS) -o m8 m8.c
m16: m16.c
	$(CC) $(CFLAGS) -o m16 m16.c
m32: m32.c
	$(CC) $(CFLAGS) -o m32 m32.c
m64: m64.c
	$(CC) $(CFLAGS) -o m64 m64.c
master: master.c
	$(CC) $(CFLAGS) -o munge master.c -lm

.PHONY : clean
clean:
	rm -f $(OBJFILES) m8 m16 m32 m64 munge$(TARGET) *~
superclean:
	rm -f $(OBJFILES) m8 m16 m32 m64 munge$(TARGET) *.csv *~
	