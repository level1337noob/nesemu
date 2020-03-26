#include <stdio.h>
#include <types.h>
#include <bus/bus.h>
#include <string.h>
#include <ppu/2c02.h>
#include <stdlib.h>


u8 Ppu::load(u16 addr)
{
	return 0;
}


void Ppu::store(u16 addr, u8 v)
{

}


void Ppu::attach_bus(Bus *bus)
{
	this->bus = bus;
}


void Ppu::power_up()
{

}
