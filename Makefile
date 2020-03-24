 # makefile for nes emulator
 
CC := g++
CFLAGS += -Igui -Igui/imgui -Igui/gui -I.\
		  -Wall -O0 -fomit-frame-pointer \
		  -fno-exceptions -march=native  \
	  	  -std=c++17 -fuse-cxa-atexit    \
	  	  -fno-exceptions -fno-rtti		 \


LOCS += -lSDL2
RELOCS += reloc/imgui_demo.o \
		  reloc/imgui_draw.o \
		  reloc/imgui_freetype.o \
		  reloc/imgui.o \
		  reloc/imgui_sdl.o \
		  reloc/imgui_widgets.o

OBJS += main.o 		  \
		init/nesemu.o \
		bus/bus.o	  \
		cpu/cpu.o	  \
		ppu/2c02.o

prog: all
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(RELOCS) $(LOCS) -lfreetype
	rm $(OBJS)

all: $(OBJS)
$(OBJS): %.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $*.o
