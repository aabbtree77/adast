# R.G. Feb 8, 2024

# fusebits: Internal 8MHz with 64ms startup time.
# disable JTAG interface as it overlaps with PORTC
# https://www.engbedded.com/fusecalc/

MCU=atmega32
CC=avr-gcc
OBJCOPY=avr-objcopy
CFLAGS=-g -mmcu=$(MCU) -Wall -Wstrict-prototypes -Os -mcall-prologues
PROG=avrdude
INTERFACE=usbasp

#-------------------
all: main.hex
main.hex : main.out 
	$(OBJCOPY) -R .eeprom -O ihex main.out main.hex
main.out : main.o
	$(CC) $(CFLAGS) -o main.out -Wl,-Map,main.map main.o
main.o : main.c
	$(CC) $(CFLAGS) -Os -c main.c
#-------------------
help: 
	@echo "Type make or make install"	
#-------------------
install:
	$(PROG) -p $(MCU) -c $(INTERFACE) -U lfuse:w:0xe4:m -U hfuse:w:0xd9:m -U flash:w:main.hex	
clean:
	rm -f *.o *.map *.out main.hex
#-------------------
