TOPDIR  := $(shell cd ..; pwd)
CC=gcc
CFLAGS=-Wall -Wextra -L. -lftd2xx -Wl,-rpath /usr/local/lib -L/usr/local/lib

APP = set_ftdi

all: $(APP)

$(APP): main.c	
	$(CC) main.c -o $(APP) $(CFLAGS)	
	
clean:
	rm -f *.o ; rm $(APP)
