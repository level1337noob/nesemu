#include <stdio.h>
#include <signal.h>
#include <types.h>
#include <init/nesemu.h>

#define DEBUG_FILE "tests/nestest/nestest.nes"

static NES_System noob_nes;
void err(int) { noob_nes.failed = true; exit(0); }

int main(int argc, char *argv[])
{
	signal(SIGINT, err);
	if (!noob_nes.load_rom(DEBUG_FILE)) {
		return 2;
	}

	noob_nes.run();
	return 0;
}
