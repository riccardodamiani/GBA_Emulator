#include "gba.h"

#undef main
int main() {
	GBA::Load("Kirby - Nightmare in Dreamland.gba");
	GBA::Run();
	return 0;
}