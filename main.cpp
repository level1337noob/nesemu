#include <stdio.h>
#include <signal.h>
#include <types.h>
#include <init/nesemu.h>

#define DEBUG_FILE "tests/nestest/nestest.nes"

static NesEmu emulator;
void err(int) { emulator.failed = true; }
int main(int argc, char *argv[])
{
	signal(SIGINT, err);
	if (!emulator.load_rom(DEBUG_FILE)) {
		return 2;
	}

	emulator.run();
	return 0;
}
