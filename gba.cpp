#include "gba.h"
#include "memory_mapper.h"
#include "cpu.h"
#include "clock.h"
#include "sound_controller.h"

#include <string>
#include <chrono>
#include <thread>

MemoryMapper GBA::memory;
Cpu GBA::cpu;
Graphics GBA::graphics;
Input GBA::input;

void GBA::Load(std::string rom_filename) {
	memory.loadRom(rom_filename);
}

void GBA::Run() {

    double totTime = 0;
    double elapsedTime = 0;
    float clock_speed = 1;
    uint32_t clks_per_second = 16'777'918;
    while (1) {
        auto startTime = std::chrono::high_resolution_clock::now();

        GBA::graphics.drawFrame();
        GBA::input.update();
        //save scancode
        if (GBA::input.wasKeyReleased(SDL_SCANCODE_F1)) {
            GBA::memory.saveState();
        }
        GBA::cpu.runFor(clock_speed * ((clks_per_second) / 60));   //  1/60th of a second
        clks_per_second -= GBA::sound.getClkAdjust()*10;  //adjust clock speed to match sound speed

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = endTime - startTime;
        elapsedTime = elapsed.count();	//elapsed time in seconds

        elapsedTime += limit_fps(elapsedTime, 60);
        totTime += elapsedTime;
    }
}

//sleeps for the time needed to have a FPS. Returns the time it have slept
double GBA::limit_fps(double elapsedTime, double maxFPS) {
    if (elapsedTime >= (1.0 / maxFPS)) {
        return 0;
    }
    else {
        std::this_thread::sleep_for(std::chrono::microseconds((long long)(((1.0 / maxFPS) - elapsedTime) * 1000000.0)));
        return ((1.0 / maxFPS) - elapsedTime);
    }
}