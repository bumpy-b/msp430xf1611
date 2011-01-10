CC=msp430-gcc
OBJCOPY=msp430-objcopy
DEBUG=-g
OPT=-Os
WARNINGS=-Wall -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wsign-compare \
		-Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused

CFLAGS=-mmcu=msp430x1611 $(OPT) $(DEBUG) -I. -I../FreeRTOS/include -I../Drivers/include -DGCC_MSP430 $(WARNINGS)

# Setup paths to source code
SOURCE_PATH = ../FreeRTOS/src
PORT_PATH = ../../Source/portable/GCC/MSP430F449
DEMO_PATH = ../Common/Minimal
COMMON_PATH = ../Common/Full
#
# Source files that can be built to THUMB mode.
#
SRC = \
main.c \
debugFunction.c \
mystdio.c \
../Drivers/src/serial.c \
../FreeRTOS/src/tasks.c \
../FreeRTOS/src/list.c \
../FreeRTOS/src/queue.c \
../FreeRTOS/src/heap_1.c \
../FreeRTOS/src/port.c \
#../FreeRTOS/src/print.c \
#
# Define all object files.
#
OBJ = $(SRC:.c=.o)

all : a.out

a.out : $(OBJ) makefile
	$(CC) $(OBJ) $(CFLAGS)

$(OBJ) : %.o : %.c makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
clean :
	del a.out
	del *.o
	








	


