PROJECT=main

OBJS = $(PROJECT).o can.o spi.o startup.o #crt0.o

CFLAGS = -Wall -fno-common -mcpu=cortex-m4 -mthumb -O2 -I./include -I.
ASFLAGS = -mcpu=cortex-m4
LDFLAGS  = -nostartfiles -Tmk20dx256.ld

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy

all:: $(PROJECT).hex

run: $(PROJECT).hex
	teensy_post_compile -file=$(PROJECT) -path=. -tools=\utils -board=TEENSY31 -reboot

$(PROJECT).hex: $(PROJECT).elf
	$(OBJCOPY) -R .stack -O ihex $(PROJECT).elf $(PROJECT).hex

$(PROJECT).elf: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(PROJECT).elf

clean:
	del *.o
	del $(PROJECT).hex
	del $(PROJECT).elf
	del $(PROJECT).map
	del $(PROJECT).bin
	del *.lst

.c.o :
	$(CC) $(CFLAGS) -c $< -o $@    

.cpp.o :
	$(CC) $(CFLAGS) -c $< -o $@

.s.o :
	$(AS) $(ASFLAGS) -o $@ $<
