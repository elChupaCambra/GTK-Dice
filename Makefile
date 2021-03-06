# change application name here (executable output name)
TARGET=dice_app

# compiler
CC=gcc
# debug
DEBUG=-g
# optimisation
OPT=-O0
# warnings
WARN=-Wall

PTHREAD=-pthread

CCFLAGS=$(DEBUG) $(OPT) $(WARN) $(PTHREAD) -pipe

GTKLIB=`pkg-config --cflags --libs gtk+-3.0`

# linker
LD=gcc
LDFLAGS=$(PTHREAD) $(GTKLIB) -export-dynamic

OBJS=	main.o	dice.o	windows.o

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)
    
main.o: src/main.c
	$(CC) -c $(CCFLAGS) src/main.c $(GTKLIB) -o main.o
	
dice.o: src/dice.c
	$(CC) -c $(CCFLAGS) src/dice.c $(GTKLIB) -o dice.o
	
windows.o: src/windows.c
	$(CC) -c $(CCFLAGS) src/windows.c $(GTKLIB) -o windows.o
	
clean:
	rm -f *.o $(TARGET)
